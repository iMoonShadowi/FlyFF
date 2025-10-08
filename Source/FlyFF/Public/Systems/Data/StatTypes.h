// OwO
#pragma once
#include "CoreMinimal.h"
#include "StatTypes.generated.h"

/** High-level buckets for inventory/org (not the DSTs themselves) */
UENUM(BlueprintType)
enum class EItemCategory : uint8
{
    Weapon, Armor, Shield, Fashion, Mount, Flight, Consumable, Quest, Material
};

/** Weapon families */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    Bow, YoYo, Knuckle, Stick, Staff, Wand, Sword1H, Sword2H, Axe1H, Axe2H
};

// Armor pieces (4)
UENUM(BlueprintType)
enum class EArmorSlot : uint8
{
    Head,
    Body,
    Gloves,
    Boots
};

// Fashion pieces (6)
UENUM(BlueprintType)
enum class EFashionSlot : uint8
{
    Head,
    Body,
    Gloves,
    Boots,
    Cloak,
    Mask
};

/** Concrete equipment slots a character can occupy */
UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
    WeaponMainHand, OffHand,
    ArmorHead, ArmorBody, ArmorGloves, ArmorBoots,
    FashionHead, FashionBody, FashionGloves, FashionBoots, FashionCloak, FashionMask,
    Mount, Flight
};

/** Elements For glows on weapons etc */
UENUM(BlueprintType)
enum class EElementalType : uint8
{
    Fire, Water, Electric, Wind, Earth
};

/** === DST: canonical stat identifiers ===
 * These encode what is being modified (similar to FlyFF DST_* codes).
 */
UENUM(BlueprintType)
enum class EStat : uint8
{
    // Primary stats
    STR, STA, DEX, INT,

    // Vitals (flat/derived)
    HP_Max, MP_Max, FP_Max,
    HP_Regen, MP_Regen, FP_Regen,

    // Offense (generic)
    AttackPower,      // base/melee/ranged consolidation
    MagicPower,       // for casters if you split
    CritChance,       // 0..1
    CritDamage,       // e.g. +0.40 = +40% crit dmg (ADoCH analogue)
    AttackSpeed,      // 0..1 (fraction of base)
    Accuracy,         // hit rate
    AdditionalDamage, // flat added to final hits

    // Defense
    Defense,          // armor/defense rating
    MagicDefense,
    DamageReduction,  // % reduction after defense (0..1)
    BlockMelee,       // % chance
    BlockRange,       // % chance
    Evasion,          // dodge chance

    // Movement/utility
    MoveSpeed,        // % as fraction (0..1)
    JumpHeight,

    // PvE/PvP scalars
    PVE_Damage,           // % dealt to monsters
    PVE_DamageTaken,      // % taken from monsters
    PVP_Damage,           // % dealt to players
    PVP_DamageTaken,      // % taken from players

    // Elemental offense/defense (percents)
    ElemDamage_Fire,
    ElemDamage_Water,
    ElemDamage_Electric,
    ElemDamage_Wind,
    ElemDamage_Earth,

    ElemResist_Fire,
    ElemResist_Water,
    ElemResist_Electric,
    ElemResist_Wind,
    ElemResist_Earth,

    // Status resists (percents)
    Resist_Stun,
    Resist_Slow,
    Resist_Silence,
    Resist_Poison,

    // Economy / meta
    DropRate,     // %
    ExpRate,      // %
    PenyaRate     // %
};

/** How a modifier applies */
UENUM(BlueprintType)
enum class EStatOp : uint8
{
    Add,         // flat add:    final = base + v
    PercentAdd,  // additive %:  final = final * (1 + sum)
    Multiplier   // multiplicative: final = final * product(1 + v)
    // (You can add Override if you need “set to X” semantics)
};

/** A single DST-style modifier (e.g. “+5 STR”, “+10% AttackSpeed”) */
USTRUCT(BlueprintType)
struct FStatModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) EStat   Stat    = EStat::STR;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) EStatOp Op      = EStatOp::Add;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float   Value   = 0.f;

    // Optional metadata for stacking/rules
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName   SourceId;   // item/buff id for dedup
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32   Priority = 0; // tiebreak for overrides
};

/** Small “base stat” block you can still use for quick items/monsters */
USTRUCT(BlueprintType)
struct FStatBlock
{
    GENERATED_BODY()

    // Keep a few commonly-edited stats for convenience
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32  Attack  = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32  Defense = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32  Vitality= 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float  CritChance = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float  CritDamage = 0.f;
};

/** Weapon-only extra bundle (optional sugar) */
USTRUCT(BlueprintType)
struct FAttackStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32  BaseAttack   = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float  AttackSpeed  = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float  Range        = 150.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool   bTwoHanded   = false;
};

/** Armor/Shield extra bundle (optional sugar) */
USTRUCT(BlueprintType)
struct FDefenseStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32  BaseDefense  = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float  BlockChance  = 0.f; // mainly shields
};
