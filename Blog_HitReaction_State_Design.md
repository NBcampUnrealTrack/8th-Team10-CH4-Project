## 개요

데미지 처리 구조를 만들고 나면 다음 문제가 생긴다.

공격이 맞는다.  
Health도 정상적으로 깎인다.  
UI도 바뀐다.

그런데 게임 화면에서는 아직 부족해 보인다.

맞은 캐릭터가 아무 반응 없이 그대로 움직이면 전투가 굉장히 가볍게 느껴진다. 액션 게임에서 피격은 단순히 체력이 줄어드는 일이 아니다. 맞은 캐릭터가 잠깐 멈추거나, 뒤로 밀려나거나, 넘어졌다가 다시 일어나는 과정까지 포함된다.

그래서 데미지 시스템 다음으로 고민한 것은 피격 상태였다.

이번 글에서는 GAS 기반 전투에서 `HitStun`, `Knockback`, `Knockdown`을 어떻게 나누어 생각했고, 각각을 어떤 흐름으로 처리했는지 정리해보려고 한다.

먼저 전체 구조를 그림으로 보면 다음과 같다. 이 글에서 계속 다룰 핵심은 `Damage`와 `HitStun`, `Knockback`, `Knockdown`을 한 덩어리로 보지 않고 역할별로 나누었다는 점이다.

<!-- 이미지 자리: 피격 처리 전체 구조도 -->
<!-- 추천 이미지: 공격 적중 이후 Damage, HitStun, Knockback, HitReact, Knockdown으로 갈라지는 다이어그램 -->

```txt
Attack Hit
    ├─ Damage
    ├─ HitStun
    ├─ Knockback
    ├─ HitReact
    └─ Knockdown
```

## 체력만 깎으면 부족했던 이유

### 데미지는 숫자고 피격은 반응이다

처음 데미지 시스템을 만들 때는 Health가 줄어드는 것만으로도 전투가 동작하는 것처럼 보였다.

```txt
공격 적중
    ↓
GE_Damage 적용
    ↓
ExecCalc_Damage 실행
    ↓
IncomingDamage 기록
    ↓
Health 감소
```

하지만 실제 플레이 감각에서는 이것만으로 부족했다.

체력은 줄었지만 캐릭터가 계속 움직일 수 있으면 공격이 맞았다는 느낌이 약하다. 특히 겟앰프드처럼 근접 액션이 중심인 게임에서는 맞았을 때의 짧은 멈춤, 밀림, 다운이 전투 리듬을 만든다.

이 부분은 글로 설명하는 것보다 실제 화면 비교가 훨씬 잘 와닿는다. 같은 데미지가 들어가더라도 캐릭터가 아무 반응 없이 서 있는 장면과, 짧게 경직되고 밀려나는 장면은 체감이 완전히 다르다.

<!-- 이미지 자리: 피격 반응이 없는 장면과 있는 장면 비교 -->
<!-- 추천 이미지: 왼쪽은 HP만 감소, 오른쪽은 HitReact/Knockback이 함께 발생하는 GIF 또는 2분할 이미지 -->

그래서 피격을 단순히 데미지의 결과로만 보지 않고, 별도의 상태 변화로 나누어 보기 시작했다.

### 공격마다 다른 피격 결과가 필요했다

피격 결과는 공격마다 다를 수 있다.

```txt
약공격
    - 낮은 데미지
    - 짧은 HitStun
    - 약한 Knockback

강공격
    - 높은 데미지
    - 긴 HitStun
    - 강한 Knockback

마무리 공격
    - Knockdown 발생
    - 일정 시간 무적
    - 넘어진 뒤 기상
```

모든 공격을 같은 방식으로 처리하면 전투가 단조로워진다.

반대로 공격마다 피격 결과를 세밀하게 바꾸려면, 데미지와 피격 상태를 함께 전달할 수 있어야 한다.

여기서부터는 “공격 하나가 단순히 데미지 숫자 하나만 가지는 게 아니라, 여러 피격 결과를 함께 가진다”는 관점으로 바뀐다.

<!-- 이미지 자리: 약공격/강공격/마무리 공격별 피격 결과 비교표 -->
<!-- 추천 이미지: 공격 타입별 BaseDamage, HitStun, Knockback, Knockdown 여부를 보여주는 표 -->

## 피격 상태를 어디에서 처리할 것인가

### 처음 떠올린 방식

가장 단순한 방식은 데미지가 들어가는 순간 모든 처리를 한 번에 하는 것이다.

```txt
Health 감소
    ↓
HitStun 적용
    ↓
Knockback 적용
    ↓
Knockdown 체크
    ↓
피격 Montage 재생
```

처음에는 이 방식이 자연스러워 보였다.

어차피 맞았을 때 한 번에 일어나는 일이기 때문이다.

하지만 구현을 진행하다 보니 각 기능의 성격이 조금씩 달랐다.

### 기능마다 성격이 달랐다

`HitStun`은 입력과 행동을 잠깐 막는 상태에 가깝다.

`Knockback`은 캐릭터를 물리적으로 밀어내는 이동 처리에 가깝다.

`Knockdown`은 넘어짐, 누워있기, 기상까지 이어지는 긴 상태 흐름에 가깝다.

이 셋을 하나의 함수에서 모두 처리하면 코드가 금방 복잡해진다.

그래서 피격 상태를 다음처럼 나누어 생각했다.

```txt
Damage
    - Health를 깎는다

HitStun
    - 잠깐 행동을 막는다

Knockback
    - 캐릭터를 밀어낸다

Knockdown
    - 넘어짐 상태로 전환한다
```

데미지는 체력 변화이고, 피격 상태는 행동 변화다.  
이 기준을 잡고 나니 각 기능을 어디에 둘지 조금 더 명확해졌다.

이 분리를 기준으로 이후 구조가 정리된다. 데미지는 GAS의 Attribute 흐름을 타고, 행동 제한은 Tag와 GameplayEffect로, 실제 밀림은 CharacterMovement 쪽으로 빠진다.

<!-- 이미지 자리: 기능별 책임 분리 다이어그램 -->
<!-- 추천 이미지: Damage=Attribute, HitStun=GameplayEffect/Tag, Knockback=CharacterMovement, Knockdown=GameplayAbility로 나누는 구조도 -->

## DamageParams에 피격 정보를 담은 이유

### 공격 결과는 ComboStep마다 다르다

콤보 시스템에서는 각 공격 단계가 별도의 데이터를 가진다.

```txt
B
BB
BBS
S
Dash B
Air S
```

각 단계마다 데미지만 다른 것이 아니라 피격 결과도 달라질 수 있다.

그래서 `ComboStep` 안에 단순 데미지 값만 두는 것으로는 부족했다.

공격 한 번의 결과를 설명하려면 다음 정보가 함께 필요했다.

```txt
BaseDamage
HitStunTime
KnockbackForce
KnockdownGroundTime
HitDirection
```

이 정보들을 따로따로 넘기면 함수 인자가 계속 늘어난다.

그래서 데미지 적용에 필요한 값을 `DamageParams` 구조체로 묶었다.

### DamageParams는 공격 결과 데이터다

`DamageParams`는 단순히 데미지 값만 담는 구조체가 아니다.

이번 공격이 적중했을 때 어떤 결과를 만들어야 하는지를 담는 데이터다.

```cpp
USTRUCT(BlueprintType)
struct FGProjectDamageEffectParams
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
    float BaseDamage = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Hit Reaction")
    FVector HitDirection = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Hit Reaction")
    FVector KnockbackForce = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hit Reaction")
    float KnockbackForceMagnitude = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hit Reaction")
    float HitstunTime = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hit Reaction")
    float KnockdownGroundTime = 0.0f;

    bool CausesKnockdown() const
    {
        return KnockdownGroundTime > 0.0f;
    }
};
```

이렇게 하면 공격 판정 코드는 타겟을 찾은 뒤, 해당 콤보 단계의 `DamageParams`를 넘기기만 하면 된다.

```txt
Attack Trace 성공
    ↓
현재 ComboStep 확인
    ↓
ComboStep의 DamageParams 사용
    ↓
타겟에게 피격 결과 적용
```

데이터가 콤보 단계에 붙어 있으니, 약공격과 강공격의 피격 결과를 에디터에서 조정하기도 쉬워진다.

실제 에디터에서는 이 구조가 꽤 중요하다. 코드에서 공격별 수치를 하드코딩하지 않고, `ComboStep` 안의 `DamageParams`를 조정하는 식으로 바뀌기 때문이다.

<!-- 이미지 자리: ComboData 에디터에서 ComboStep과 DamageParams가 보이는 화면 -->
<!-- 추천 이미지: DA_ComboData의 ComboSteps 배열, DamageParams 펼친 디테일 패널 캡처 -->

## HitStun 설계

### HitStun이 필요한 이유

HitStun은 맞은 캐릭터가 일정 시간 행동하지 못하게 만드는 상태다.

전투에서 HitStun이 없으면 공격을 맞아도 바로 반격하거나 움직일 수 있다. 그러면 공격을 맞춘 쪽의 손맛이 약해지고, 콤보 연결도 어색해진다.

그래서 피격 시 짧은 시간 동안 캐릭터의 행동을 막는 상태가 필요했다.

```txt
공격 적중
    ↓
HitStun 적용
    ↓
일정 시간 입력/행동 제한
    ↓
HitStun 종료
```

### HitStun을 태그로 표현하기

GAS에서는 상태를 GameplayTag로 표현하기 좋다.

그래서 HitStun 상태는 태그로 관리했다.

```txt
State.Combat.HitStun
```

이 태그가 있는 동안에는 공격, 이동, 대시 같은 행동을 제한할 수 있다.

핵심은 HitStun을 bool 변수 하나로만 보지 않았다는 점이다.

태그로 두면 Ability의 Activation 조건, Blocked Tag, Cancel Tag와 연결하기 쉽다.

```txt
HitStun 태그 보유
    ↓
공격 Ability 실행 불가
    ↓
Sprint Ability 실행 불가
    ↓
필요한 Ability 취소
```

### HitStun은 별도 GameplayEffect로 처리했다

HitStun은 일정 시간이 지나면 사라져야 한다.

그래서 Duration을 가진 GameplayEffect로 처리하는 방식이 어울렸다.

```txt
GE_HitStun
    - Duration 있음
    - State.Combat.HitStun 태그 부여
```

공격이 적중했을 때 `HitStunTime`이 0보다 크면, 해당 시간만큼 HitStun GameplayEffect를 적용한다.

```cpp
void UGProjectAbilitySystemLibrary::ApplyHitstunEffect(
    const FGProjectDamageEffectParams& DamageEffectParams,
    TSubclassOf<UGameplayEffect> HitstunEffectClass)
{
    UAbilitySystemComponent* SourceASC = DamageEffectParams.SourceAbilitySystemComponent;
    UAbilitySystemComponent* TargetASC = DamageEffectParams.TargetAbilitySystemComponent;
    if (!SourceASC || !TargetASC || !HitstunEffectClass || DamageEffectParams.HitstunTime <= 0.0f)
    {
        return;
    }

    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
        HitstunEffectClass,
        1.0f,
        SourceASC->MakeEffectContext());

    if (!SpecHandle.IsValid())
    {
        return;
    }

    SpecHandle.Data->SetDuration(DamageEffectParams.HitstunTime, true);
    TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
```

이 방식은 명확하다.

시간이 있는 상태는 Duration GameplayEffect로 처리한다.  
행동 제한은 GameplayTag로 처리한다.

이 구조는 블루프린트 에셋 화면을 같이 보여주면 이해가 훨씬 쉽다. `GE_HitStun`이 Duration을 가지고 있고, 그 동안 `State.Combat.HitStun` 태그를 부여한다는 점이 핵심이다.

<!-- 이미지 자리: GE_HitStun 설정 화면 -->
<!-- 추천 이미지: Duration Policy, Duration Magnitude, Granted Tags에 State.Combat.HitStun이 들어간 화면 -->

## HitReact 설계

### HitStun과 HitReact는 다르다

처음에는 HitStun이 들어오면 바로 피격 애니메이션을 재생하면 된다고 생각했다.

하지만 HitStun과 HitReact는 역할이 조금 다르다.

`HitStun`은 행동 제한 상태다.  
`HitReact`는 화면에 보이는 피격 반응이다.

둘을 완전히 같은 것으로 묶으면 나중에 문제가 생긴다.

예를 들어 아주 짧은 HitStun은 있지만 별도의 피격 모션은 생략하고 싶을 수 있다. 반대로 큰 공격은 HitStun과 함께 방향별 피격 Montage를 재생해야 할 수도 있다.

그래서 HitReact는 별도의 반응 처리로 보는 것이 좋다고 판단했다.

### 방향별 Section을 사용한 이유

피격 모션은 방향에 따라 달라질 수 있다.

```txt
앞에서 맞음
뒤에서 맞음
왼쪽에서 맞음
오른쪽에서 맞음
```

이때 Montage를 여러 개로 나누는 방법도 있지만, 하나의 Montage 안에서 Section을 나누는 방식이 더 단순했다.

```txt
AM_HitReact
    - Front
    - Back
    - Left
    - Right
```

공격 방향을 계산한 뒤, 해당 방향에 맞는 Section을 재생한다.

```txt
공격 방향 계산
    ↓
Front / Back / Left / Right 결정
    ↓
HitReact Montage Section 재생
```

이렇게 하면 애니메이션 에셋 관리가 단순해진다.

여기에는 Montage Section 화면을 넣기 좋다. 글로는 `Front`, `Back`, `Left`, `Right`라고 설명했지만, 실제 Montage 타임라인에서 Section이 나뉘어 있는 모습을 보면 훨씬 직관적이다.

<!-- 이미지 자리: AM_HitReact Montage Section 화면 -->
<!-- 추천 이미지: HitReact Montage 안에 Front/Back/Left/Right Section이 배치된 캡처 -->

## Knockback 설계

### Knockback은 데미지 계산과 다른 문제였다

Knockback은 맞은 캐릭터를 뒤로 밀어내는 처리다.

처음에는 데미지 후처리에서 같이 처리하면 될 것처럼 보였다.

```txt
Health 감소
    ↓
Knockback 적용
```

하지만 생각해보면 Knockback은 Attribute 계산이 아니라 캐릭터 이동 처리에 가깝다.

데미지는 GAS의 Attribute 흐름으로 처리하지만, 실제 캐릭터를 밀어내는 것은 CharacterMovement와 연결된다.

그래서 Knockback을 단순히 ExecCalc 안에서 처리하는 것은 어색했다.

ExecCalc는 계산기다.  
캐릭터를 Launch하거나 이동시키는 것은 캐릭터 쪽 책임에 가깝다.

### Knockback 방향과 세기

Knockback에서 가장 먼저 필요한 것은 방향이다.

일반적으로는 공격자에서 피격자로 향하는 방향을 사용한다.

```txt
KnockbackDirection = TargetLocation - SourceLocation
```

그리고 탑다운 근접 액션에서는 보통 수평 방향이 더 중요하다.

```txt
Direction.Z = 0
Normalize
```

이 방향에 KnockbackForce를 곱하면 최종 밀림 벡터를 만들 수 있다.

```cpp
void UGProjectAbilitySystemLibrary::SetKnockbackDirection(
    FGProjectDamageEffectParams& DamageEffectParams,
    FVector KnockbackDirection,
    float Magnitude)
{
    if (KnockbackDirection.IsNearlyZero())
    {
        DamageEffectParams.HitDirection = FVector::ZeroVector;
        DamageEffectParams.KnockbackForce = FVector::ZeroVector;
        return;
    }

    KnockbackDirection.Z = 0.0f;
    KnockbackDirection.Normalize();

    DamageEffectParams.HitDirection = KnockbackDirection;

    const float BaseMagnitude = Magnitude > 0.0f
        ? Magnitude
        : DamageEffectParams.KnockbackForceMagnitude;

    DamageEffectParams.KnockbackForce = KnockbackDirection * BaseMagnitude;
}
```

실제 프로젝트에서는 여기에 KnockbackPower, KnockbackResistance 같은 Attribute를 더해 최종 세기를 보정했다.

### KnockbackForce는 콤보 데이터에서 조정한다

Knockback 세기는 공격마다 다르다.

약공격은 조금만 밀리고, 강공격이나 마무리 공격은 크게 밀려야 한다.

그래서 KnockbackForce는 코드에 하드코딩하지 않고, `DamageParams` 또는 콤보 단계 데이터에서 조정할 수 있게 두었다.

```txt
B
    KnockbackForce = 100

S
    KnockbackForce = 300

BBS
    KnockbackForce = 600
```

이렇게 하면 에디터에서 공격별 손맛을 조정할 수 있다.

### Knockback 적용 시점

Knockback은 공격이 실제로 맞는 순간 적용되어야 한다.

공격 버튼을 눌렀을 때도 아니고, Montage가 시작될 때도 아니다.

AnimNotifyState 구간에서 Trace가 타겟을 감지했을 때가 적절하다.

```txt
공격 Montage 재생
    ↓
AttackTrace NotifyState 시작
    ↓
Trace Tick
    ↓
타겟 감지
    ↓
Damage 적용
    ↓
Knockback 적용
```

이 기준을 잡아두면 공격 애니메이션의 타격 타이밍과 실제 피격 결과가 맞아떨어진다.

이 부분은 디버그 Trace 캡처를 넣기 좋은 지점이다. 공격 판정이 버튼 입력 시점이 아니라, AnimNotifyState 구간에서 실제 무기 또는 손 소켓을 따라가며 발생한다는 것을 보여줄 수 있다.

<!-- 이미지 자리: 공격 Trace 디버그 화면 -->
<!-- 추천 이미지: 초록/빨간 Trace 캡슐이 무기 궤적을 따라가는 장면, 맞았을 때 빨간색으로 바뀌는 장면 -->

## Knockback 디버깅 과정

### 수치는 들어가는데 밀리지 않는 문제

Knockback을 구현하면서 헷갈렸던 부분이 있었다.

로그상으로는 Force가 계산되고 있었다.

```txt
Base=1000
Direction=(-0.99, 0.15, 0)
Force=(-989, 146, 0)
```

하지만 실제 캐릭터의 Velocity는 0으로 남아 있었다.

이런 경우에는 단순히 Knockback 코드만 볼 것이 아니라 CharacterMovement 상태까지 같이 봐야 한다.

```txt
MovementMode
GroundFriction
BrakingDeceleration
RootMotion
Montage 설정
서버 권한
CharacterMovement가 Launch를 받는 상태인지
```

넉백은 숫자 계산보다 적용 환경이 중요했다.

이때 남긴 로그나 디버그 화면을 넣으면 글의 설득력이 좋아진다. 단순히 “안 됐다”가 아니라, 계산값은 있었는데 CharacterMovement 쪽에서 실제 이동이 일어나지 않았다는 흐름을 보여줄 수 있기 때문이다.

<!-- 이미지 자리: Knockback 디버깅 로그 또는 Before/After 비교 -->
<!-- 추천 이미지: 계산된 Force 로그, 캐릭터 Velocity 로그, 실제 밀림 전후 화면 -->

### 애니메이션과 이동의 충돌

HitReact나 Knockdown Montage에 RootMotion이 강하게 들어가 있으면, Knockback으로 밀어낸 이동과 애니메이션 이동이 충돌할 수 있다.

그래서 피격 반응 Montage에서는 RootMotion 사용 여부를 신중하게 봐야 했다.

이 프로젝트에서는 Knockback은 코드에서 밀어내고, 피격 애니메이션은 반응만 담당하는 방향이 더 단순했다.

```txt
Knockback
    - CharacterMovement가 담당

HitReact Montage
    - 시각적 반응 담당
```

둘의 책임을 나누니 디버깅하기 쉬워졌다.

## Knockdown 설계

### Knockdown은 긴 피격 상태다

Knockdown은 단순히 잠깐 멈추는 HitStun과 다르다.

캐릭터가 넘어지고, 바닥에 누워 있고, 다시 일어나는 흐름이 필요하다.

```txt
넘어짐
    ↓
누워있기
    ↓
기상
```

이 흐름은 짧은 상태 태그 하나로 끝내기 어렵다.

그래서 Knockdown은 하나의 상태 흐름으로 보았다.

Knockdown은 글보다 그림이 훨씬 잘 먹히는 파트다. `Fall -> DownLoop -> GetUp` 흐름을 타임라인으로 그려두면, 왜 단순 HitStun과 다르게 봤는지 바로 이해된다.

<!-- 이미지 자리: Knockdown 상태 타임라인 -->
<!-- 추천 이미지: Fall, DownLoop, GetUp Section과 GroundTime이 표시된 타임라인 다이어그램 -->

### Knockdown을 세 단계로 나누기

Knockdown은 크게 세 단계로 나눌 수 있다.

```txt
Fall
    - 넘어지는 애니메이션

DownLoop
    - 바닥에 누워있는 상태

GetUp
    - 일어나는 애니메이션
```

넘어지는 시간과 일어나는 시간은 애니메이션 길이에 의해 거의 고정된다.

반면 바닥에 누워있는 시간은 공격마다 다르게 줄 수 있다.

그래서 별도의 Knockdown 총 시간을 두기보다, `GroundTime`을 조정하는 방식이 더 직관적이었다.

```txt
Fall Montage 재생
    ↓
DownLoop 상태 유지
    ↓
GroundTime만큼 대기
    ↓
GetUp 재생
```

### KnockdownGroundTime을 사용한 이유

처음에는 Knockdown 여부를 bool로 둘 수도 있었다.

```txt
bKnockdown = true
```

하지만 곧 불필요하다고 느꼈다.

어차피 누워있는 시간이 0보다 크면 Knockdown이 발생한다고 볼 수 있다.

```txt
KnockdownGroundTime <= 0
    - Knockdown 없음

KnockdownGroundTime > 0
    - Knockdown 발생
```

그래서 bool을 따로 두기보다 GroundTime 값으로 판단하는 구조가 더 단순했다.

## Knockdown과 무적 상태

### 누워있는 동안 계속 맞아야 하는가

Knockdown을 만들면서 또 하나의 문제가 생겼다.

넘어진 캐릭터를 계속 때릴 수 있게 할 것인가?

프로토타입에서는 누워있는 동안 계속 맞으면 전투가 너무 지저분해질 수 있다고 판단했다. 특히 아직 기상 공격이나 다운 공격 시스템이 없는 상태에서는, 다운된 캐릭터가 계속 맞는 것보다 일정 시간 무적으로 두는 편이 안정적이다.

그래서 Knockdown 중에는 무적 상태를 함께 부여하기로 했다.

```txt
Knockdown 시작
    ↓
State.Combat.Knockdown 부여
    ↓
State.Combat.Invincible 부여
    ↓
GroundTime 이후 기상
    ↓
태그 제거
```

### Invincible 태그를 따로 둔 이유

Knockdown 상태 자체를 보고 데미지를 막을 수도 있다.

하지만 무적은 Knockdown 외에도 다른 곳에서 사용할 수 있다.

```txt
부활 직후 무적
회피 중 무적
스폰 보호
특수 스킬 무적
Knockdown 중 무적
```

그래서 무적은 별도 태그로 분리하는 것이 좋았다.

```txt
State.Combat.Invincible
```

이 태그가 있는 대상은 데미지 적용 대상에서 제외하거나, 데미지를 0으로 처리할 수 있다.

## Knockdown을 GA로 처리한 이유

### 상태 흐름이 있기 때문이다

Knockdown은 단순히 태그 하나를 붙였다가 지우는 기능이 아니다.

```txt
Montage 재생
Section 전환
GroundTime 대기
기상 Montage 재생
입력 제한
무적 처리
종료 후 상태 복구
```

이런 흐름은 Gameplay Ability로 처리하기 좋다.

Ability는 시작, 대기, 종료 흐름을 명확하게 가질 수 있고, 중간에 Montage나 WaitTask를 붙이기도 쉽다.

그래서 Knockdown은 별도의 Ability로 두는 방향이 적절했다.

### Startup Ability로 미리 등록하기

Knockdown Ability는 입력으로 발동하는 기술이 아니다.

피격 결과로 발동되어야 한다.

그래서 캐릭터가 시작할 때 미리 Ability를 부여해두고, Knockdown 이벤트가 들어오면 실행되는 방식으로 구성할 수 있다.

```txt
Character BeginPlay / ASC Init
    ↓
GA_Knockdown 부여
    ↓
피격 시 Knockdown 이벤트 발생
    ↓
GA_Knockdown 활성화
```

이 방식은 HitReact나 Death 같은 반응형 Ability에도 응용할 수 있다.

여기서는 GA_Knockdown의 이벤트 기반 실행 흐름을 보여주면 좋다. 입력으로 누르는 Ability가 아니라, 피격 이벤트가 들어왔을 때 반응하는 Ability라는 점이 핵심이다.

<!-- 이미지 자리: GA_Knockdown 이벤트 실행 흐름 -->
<!-- 추천 이미지: Event.Combat.Knockdown 수신 -> Montage Fall -> Wait GroundTime -> GetUp -> EndAbility 흐름도 -->

## 실전에서 추가로 고려한 문제들

### 한 공격에서 같은 대상을 여러 번 맞히지 않기

NotifyState 기반 Trace는 한 프레임만 검사하는 것이 아니라, 공격 판정 구간 동안 계속 Trace를 수행한다.

이때 아무 처리도 하지 않으면 같은 공격 한 번에 같은 대상이 여러 번 맞을 수 있다.

그래서 현재 공격 단계에서 이미 맞은 대상을 기록해두고, 같은 Step 안에서는 다시 데미지를 적용하지 않도록 했다.

```cpp
for (const FHitResult& HitResult : HitResults)
{
    AActor* Target = HitResult.GetActor();
    const TWeakObjectPtr<AActor> TargetPtr(Target);

    if (!Target || HitActorsThisStep.Contains(TargetPtr))
    {
        continue;
    }

    ApplyDamageToTarget(Target);

    HitActorsThisStep.Add(TargetPtr);
}
```

이 처리는 단순하지만 중요했다.

공격 판정이 연속 Trace로 바뀌는 순간, 중복 피격 방지를 하지 않으면 데미지가 의도보다 훨씬 많이 들어갈 수 있기 때문이다.

이 부분은 짧은 GIF나 연속 프레임 캡처가 잘 어울린다. 같은 공격 판정 구간에서 Trace가 여러 번 지나가지만, 실제 데미지는 한 번만 들어가는 모습을 보여주면 된다.

<!-- 이미지 자리: 중복 피격 방지 예시 -->
<!-- 추천 이미지: 같은 공격 구간에서 Trace는 여러 번 발생하지만 Damage Text는 한 번만 뜨는 장면 -->

### 너무 짧은 간격의 재피격도 막아야 했다

한 Step 안에서 중복 피격을 막는 것만으로 부족한 경우도 있다.

예를 들어 여러 개의 Hit Window가 짧은 간격으로 이어지거나, 같은 Montage 안에서 Trace가 겹치면 거의 같은 순간에 두 번 맞는 상황이 생길 수 있다.

그래서 마지막 피격 시간을 기록하고, 너무 짧은 시간 안에 같은 대상이 다시 맞으면 무시하는 보호 장치를 둘 수 있다.

```cpp
const float CurrentTime = GetWorld()->GetTimeSeconds();
if (const float* LastHitTime = LastHitTimestamps.Find(TargetPtr);
    LastHitTime && CurrentTime - *LastHitTime < MinHitReapplyInterval)
{
    continue;
}

LastHitTimestamps.Add(TargetPtr, CurrentTime);
```

이런 코드는 화려한 기능은 아니지만, 액션 게임 판정을 안정적으로 만드는 데 꽤 큰 역할을 한다.

### 멀티플레이어에서는 서버 판정을 기준으로 둔다

피격 처리는 클라이언트가 임의로 결정하면 안 된다.

공격이 맞았는지, 데미지가 들어갔는지, Knockdown이 발생했는지는 서버가 확정해야 한다.

클라이언트는 그 결과를 받아 HitReact Montage, UI, 이펙트 같은 시각적 반응을 재생한다.

그래서 이 구조에서는 데미지 적용과 상태 부여는 서버 기준으로 처리하고, 화면 피드백은 Replication 또는 Multicast를 통해 전달하는 방향으로 잡았다.

```txt
Client
    - 입력
    - 예측 가능한 애니메이션 재생

Server
    - 공격 판정 확정
    - DamageParams 적용
    - HitStun / Knockdown 상태 확정

Clients
    - HitReact
    - UI
    - VFX / SFX
```

이 기준을 잡아두면 나중에 피격 판정이 흔들리거나 클라이언트마다 결과가 달라지는 문제를 줄일 수 있다.

멀티플레이어 부분은 서버와 클라이언트 화면을 나란히 보여주면 좋다. 서버가 판정을 확정하고, 클라이언트는 결과를 받아 피격 반응을 재생한다는 구조가 한눈에 들어온다.

<!-- 이미지 자리: 서버 판정과 클라이언트 피드백 구조 -->
<!-- 추천 이미지: Server 화면/Client 화면 비교, 또는 Server Authority -> Clients Feedback 다이어그램 -->

### GameplayCue로 처리하는 방법도 있었다

VFX, SFX, CameraShake 같은 순수 피드백은 GameplayCue로 분리할 수도 있다.

이 방식은 피격 이펙트를 GAS 흐름 안에서 일관되게 관리할 수 있다는 장점이 있다.

```txt
GE_Damage 적용
    ↓
GameplayCue.HitImpact 실행
    ↓
VFX / SFX / CameraShake 재생
```

다만 이번 구조에서는 먼저 피격 상태 자체를 나누는 것이 더 중요했다.

HitStun, Knockback, Knockdown은 단순한 이펙트가 아니라 캐릭터의 행동과 이동에 영향을 준다. 그래서 피격 상태 흐름은 C++과 GA 중심으로 잡고, VFX/SFX는 이후 GameplayCue로 확장할 수 있는 지점으로 남겨두는 편이 낫다고 판단했다.

## 피격 애니메이션은 어디서 실행할 것인가

### 선택지는 여러 가지였다

피격 애니메이션을 처리하는 방법은 여러 가지가 있다.

```txt
Character 함수에서 바로 Montage 재생
Gameplay Ability에서 Montage 재생
AnimBP 상태머신에서 태그를 보고 전환
GameplayCue로 VFX/SFX와 함께 처리
```

각 방식마다 장단점이 있다.

### Character에서 바로 재생하는 방식

가장 단순한 방식은 캐릭터 함수에서 바로 Montage를 재생하는 것이다.

```txt
피격 발생
    ↓
Character->PlayHitReactMontage()
```

빠르게 만들 수 있지만, 상태가 많아질수록 Character 클래스가 무거워진다.

### AnimBP 상태머신 방식

AnimBP에서 태그나 변수 상태를 보고 HitReact 상태로 전환할 수도 있다.

이 방식은 애니메이션 제어를 AnimBP 쪽에 맡길 수 있다는 장점이 있다.

하지만 순간적인 피격 반응이나 방향별 Section 재생에는 Montage 방식이 더 직관적이었다.

### Gameplay Ability 방식

HitReact나 Knockdown을 Ability로 처리하면 GAS의 태그, 취소, Block 조건과 연결하기 쉽다.

특히 Knockdown처럼 시간 흐름이 있는 상태는 Ability로 처리하는 편이 자연스러웠다.

그래서 이 프로젝트에서는 단순 피격 반응은 가볍게 처리하되, Knockdown처럼 흐름이 긴 상태는 GA로 처리하는 쪽이 적절하다고 판단했다.

## 최종 피격 흐름

### 전체 흐름

최종적으로 피격 상태는 다음 흐름으로 정리할 수 있다.

```txt
공격 Trace 성공
    ↓
타겟이 무적 상태인지 확인
    ↓
DamageParams 확인
    ↓
GE_Damage 적용
    ↓
Health 감소
    ↓
HitStunTime > 0 이면 GE_HitStun 적용
    ↓
KnockbackForce > 0 이면 Knockback 적용
    ↓
HitReact Montage 재생
    ↓
KnockdownGroundTime > 0 이면 Knockdown Ability 실행
```

이 흐름에서 중요한 점은 모든 피격 결과가 하나의 함수에 뭉쳐 있지 않다는 것이다.

데미지는 데미지 시스템이 처리한다.  
HitStun은 Duration GameplayEffect가 처리한다.  
Knockback은 CharacterMovement 기반으로 처리한다.  
Knockdown은 Ability 흐름으로 처리한다.  
애니메이션은 Montage Section으로 표현한다.

### 데이터 기준 흐름

콤보 데이터 관점에서는 다음처럼 볼 수 있다.

```txt
ComboStep
    ↓
DamageParams
    ↓
BaseDamage
HitStunTime
KnockbackForce
KnockdownGroundTime
```

즉 콤보 단계 하나가 데미지와 피격 결과를 함께 가진다.

공격 로직은 이 데이터를 읽어서 적중 대상에게 적용한다.

최종 흐름은 글의 앞부분에서 본 구조와 다시 연결된다. 처음에는 단순히 `Health 감소` 하나로 보였던 피격 처리가, 이제는 데이터와 상태 흐름으로 나누어진다.

<!-- 이미지 자리: 최종 피격 처리 플로우 요약 -->
<!-- 추천 이미지: ComboStep -> DamageParams -> Damage/HitStun/Knockback/HitReact/Knockdown으로 이어지는 최종 구조도 -->

## 구현하면서 헷갈렸던 지점

### Knockback을 ExecCalc에서 해야 하는가

처음에는 Knockback도 데미지 계산의 일부처럼 보였다.

하지만 Knockback은 Attribute 계산이라기보다 캐릭터 이동 처리다.

그래서 ExecCalc에서 직접 CharacterMovement를 건드리는 것은 책임이 맞지 않다고 느꼈다.

ExecCalc는 계산만 한다.  
캐릭터를 실제로 밀어내는 것은 피격 후처리 쪽에서 처리한다.

### Knockdown과 HitStun은 같은가

둘 다 피격 상태이지만 성격이 다르다.

HitStun은 짧은 행동 제한이다.  
Knockdown은 넘어짐부터 기상까지 이어지는 상태 흐름이다.

그래서 HitStun은 Duration GameplayEffect가 어울렸고, Knockdown은 Gameplay Ability가 더 어울렸다.

### Knockdown bool이 필요한가

처음에는 Knockdown 여부를 bool로 둘 수 있다고 생각했다.

하지만 `GroundTime`이 0보다 크면 Knockdown이라고 판단할 수 있다.

그래서 bool을 따로 두지 않는 편이 더 단순했다.

### 무적은 Knockdown 안에 넣을 것인가

Knockdown 중 무적이 필요하긴 했지만, 무적이라는 개념은 Knockdown 전용이 아니다.

그래서 `State.Combat.Invincible` 태그를 별도로 두었다.

이렇게 해두면 나중에 회피, 부활, 스폰 보호에도 같은 태그를 사용할 수 있다.

## 최종 정리

피격 상태를 만들면서 가장 크게 느낀 점은 데미지와 피격 반응을 같은 것으로 보면 안 된다는 것이었다.

데미지는 Health를 줄이는 숫자 처리다.  
피격 상태는 캐릭터의 행동과 화면 반응을 바꾸는 상태 처리다.

이 둘을 분리하니 구조가 훨씬 명확해졌다.

```txt
Damage
    - Health 감소

HitStun
    - 짧은 행동 제한

Knockback
    - 물리적 밀림

HitReact
    - 시각적 피격 반응

Knockdown
    - 넘어짐, 누워있기, 기상 흐름

Invincible
    - 데미지 무시 상태
```

처음에는 공격이 맞았을 때 모든 처리를 한 번에 넣으면 될 것 같았다.

하지만 HitStun, Knockback, Knockdown을 추가하면서 각 기능의 성격이 다르다는 것을 알게 되었다.

그래서 시간 기반 상태는 GameplayEffect로, 긴 상태 흐름은 Gameplay Ability로, 실제 밀림은 CharacterMovement로, 시각적 반응은 Montage Section으로 나누었다.

여기에 중복 피격 방지, 서버 판정 기준, GameplayCue 확장 가능성까지 고려하니 피격 구조가 조금 더 안정적으로 정리됐다.

이 방식이 완벽하다고 말하기는 어렵다.

다만 구현해보니 무엇을 어디에서 고쳐야 하는지는 꽤 명확해졌다. 데미지를 바꾸고 싶으면 `DamageParams`를 보면 되고, 상태 시간을 바꾸고 싶으면 GameplayEffect나 Knockdown Ability를 보면 된다. 피격 연출을 바꾸고 싶다면 Montage Section이나 이후 GameplayCue 쪽으로 확장하면 된다.

그 정도만 되어도 프로토타입 단계에서는 충분히 괜찮은 구조라고 느꼈다.
