import wd = require("TypeScript/wd")
import _ge = require("Scripting/GameEnums")

export class MsgAddConsumable extends wd.Message {

    WD_DECLARE_MESSAGE_TYPE;

    consumableType: _ge.Consumable;
    amount: number = 0;
    return_consumed: boolean = true;
}

export class MsgUnlockWeapon extends wd.Message {

    WD_DECLARE_MESSAGE_TYPE;

    WeaponType: _ge.Weapon;
    return_consumed: boolean = true;
}