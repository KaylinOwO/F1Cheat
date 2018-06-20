namespace Knife.Enums.TF {
    enum PlayerExtraCondition {
        SpeedBuffAlly = 1 << 0, //Toggled when a player gets hit with the disciplinary action. 
        HalloweenCritCandy = 1 << 1, //Only for Scream Fortress event maps that drop crit candy. 
        CritCanteen = 1 << 2, //Player is getting a crit boost from a canteen.
        CritHype = 1 << 4, //Soda Popper crits. 
        CritOnFirstBlood = 1 << 5, //Arena first blood crit buff. 
        CritOnWin = 1 << 6, //End of round crits. 
        CritOnFlagCapture = 1 << 7, //CTF intelligence capture crits. 
        CritOnKill = 1 << 8, //Unknown what this is for. 
        RestrictToMelee = 1 << 9, //Unknown what this is for. 
        Reprogrammed = 1 << 11, //MvM Bot has been reprogrammed.
        PyroCrits = 1 << 12, //Player is getting crits from the Mmmph charge. 
        PyroHeal = 1 << 13, //Player is being healed from the Mmmph charge. 
        FocusBuff = 1 << 14, //Player is getting a focus buff.
        DisguisedRemoved = 1 << 15, //Disguised remove from a bot.
        MarkedForDeathSilent = 1 << 16, //MvM related.
        DisguisedAsDispenser = 1 << 17, //Bot is disguised as dispenser.
        Sapped = 1 << 18, //MvM bot is being sapped.
        UberchargedHidden = 1 << 19, //MvM Related
        UberchargedCanteen = 1 << 20, //Player is recieveing ubercharge from a canteen.
        HalloweenBombHead = 1 << 21, //Player has a bomb on their head from Merasmus.
        HalloweenThriller = 1 << 22, //Players are forced to dance from Merasmus.
    }
}
