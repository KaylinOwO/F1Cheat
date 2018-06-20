using System;

namespace Knife.Enums.TF {
    [Flags]
    enum PlayerCondition {
        Slowed = 1 << 0, //Toggled when a player is slowed down. 
        Zoomed = 1 << 1, //Toggled when a player is zoomed. 
        Disguising = 1 << 2, //Toggled when a Spy is disguising.  
        Disguised = 1 << 3, //Toggled when a Spy is disguised. 
        Cloaked = 1 << 4, //Toggled when a Spy is invisible. 
        Ubercharged = 1 << 5, //Toggled when a player is ÜberCharged. 
        TeleportedGlow = 1 << 6, //Toggled when someone leaves a teleporter and has glow beneath their feet. 
        Taunting = 1 << 7, //Toggled when a player is taunting. 
        UberchargeFading = 1 << 8, //Toggled when the ÜberCharge is fading. 
        CloakFlicker = 1 << 9, //Toggled when a Spy is visible during cloak. 
        Teleporting = 1 << 10, //Only activates for a brief second when the player is being teleported; not very useful. 
        Kritzkrieged = 1 << 11, //Toggled when a player is being crit buffed by the KritzKrieg. 
        TmpDamageBonus = 1 << 12, //Unknown what this is for. Name taken from the AlliedModders SDK. 
        DeadRingered = 1 << 13, //Toggled when a player is taking reduced damage from the Deadringer. 
        Bonked = 1 << 14, //Toggled when a player is under the effects of The Bonk! Atomic Punch. 
        Stunned = 1 << 15, //Toggled when a player's speed is reduced from airblast or a Sandman ball. 
        Buffed = 1 << 16, //Toggled when a player is within range of an activated Buff Banner. 
        Charging = 1 << 17, //Toggled when a Demoman charges with the shield. 
        DemoBuff = 1 << 18, //Toggled when a Demoman has heads from the Eyelander. 
        CritCola = 1 << 19, //Toggled when the player is under the effect of The Crit-a-Cola. 
        InHealRadius = 1 << 20, //Unused condition, name taken from AlliedModders SDK. 
        Healing = 1 << 21, //Toggled when someone is being healed by a medic or a dispenser. 
        OnFire = 1 << 22, //Toggled when a player is on fire. 
        Overhealed = 1 << 23, //Toggled when a player has >100% health. 
        Jarated = 1 << 24, //Toggled when a player is hit with a Sniper's Jarate. 
        Bleeding = 1 << 25, //Toggled when a player is taking bleeding damage. 
        DefenseBuffed = 1 << 26, //Toggled when a player is within range of an activated Battalion's Backup. 
        Milked = 1 << 27, //Player was hit with a jar of Mad Milk. 
        MegaHeal = 1 << 28, //Player is under the effect of Quick-Fix charge. 
        RegenBuffed = 1 << 29, //Toggled when a player is within a Concheror's range. 
        MarkedForDeath = 1 << 30, //Player is marked for death by a Fan O'War hit. Effects are similar to _Jarated. 
    }
}
