import wd = require("TypeScript/wd")

import _ge = require("Scripting/GameEnums")
import _gm = require("Scripting/GameMessages")
import _guns = require("Prefabs/Guns/Gun")

export class Player extends wd.TickedTypescriptComponent {

    /* BEGIN AUTO-GENERATED: VARIABLES */
    GiveAllWeapons: boolean = false;
    Invincible: boolean = false;
    /* END AUTO-GENERATED: VARIABLES */

    constructor() {
        super()
    }

    characterController: wd.JoltDefaultCharacterComponent = null;
    camera: wd.GameObject = null;
    input: wd.InputComponent = null;
    headBone: wd.HeadBoneComponent = null;
    gunRoot: wd.GameObject = null;
    flashlightObj: wd.GameObject = null;
    flashlight: wd.SpotLightComponent = null;
    activeWeapon: _ge.Weapon = _ge.Weapon.None;
    holsteredWeapon: _ge.Weapon = _ge.Weapon.None;
    guns: wd.GameObject[] = [];
    gunComp: _guns.Gun[] = [];
    ammoPouch: _guns.AmmoPouch = new _guns.AmmoPouch();
    weaponUnlocked: boolean[] = [];
    grabObject: wd.JoltGrabObjectComponent = null;
    requireNoShoot: boolean = false;
    blackboard: wd.BlackboardComponent = null;

    OnSimulationStarted(): void {
        let owner = this.GetOwner();
        this.characterController = owner.TryGetComponentOfBaseType(wd.JoltDefaultCharacterComponent);
        this.camera = owner.FindChildByName("Camera", true);
        this.input = owner.TryGetComponentOfBaseType(wd.InputComponent);
        this.headBone = this.camera.TryGetComponentOfBaseType(wd.HeadBoneComponent);
        this.gunRoot = owner.FindChildByName("Gun", true);
        this.flashlightObj = owner.FindChildByName("Flashlight", true);
        this.flashlight = this.flashlightObj.TryGetComponentOfBaseType(wd.SpotLightComponent);
        this.guns[_ge.Weapon.Pistol] = wd.Utils.FindPrefabRootNode(this.gunRoot.FindChildByName("Pistol", true));
        this.guns[_ge.Weapon.Shotgun] = wd.Utils.FindPrefabRootNode(this.gunRoot.FindChildByName("Shotgun", true));
        this.guns[_ge.Weapon.MachineGun] = wd.Utils.FindPrefabRootNode(this.gunRoot.FindChildByName("MachineGun", true));
        this.guns[_ge.Weapon.PlasmaRifle] = wd.Utils.FindPrefabRootNode(this.gunRoot.FindChildByName("PlasmaRifle", true));
        this.guns[_ge.Weapon.RocketLauncher] = wd.Utils.FindPrefabRootNode(this.gunRoot.FindChildByName("RocketLauncher", true));
        this.blackboard = owner.TryGetComponentOfBaseType(wd.BlackboardComponent);

        this.grabObject = owner.FindChildByName("GrabObject", true).TryGetComponentOfBaseType(wd.JoltGrabObjectComponent);
        this.SetTickInterval(wd.Time.Milliseconds(0));

        this.weaponUnlocked[_ge.Weapon.None] = true;
        this.weaponUnlocked[_ge.Weapon.Pistol] = true;

        if (this.GiveAllWeapons) {

            this.weaponUnlocked[_ge.Weapon.PlasmaRifle] = true;
            this.weaponUnlocked[_ge.Weapon.MachineGun] = true;
            this.weaponUnlocked[_ge.Weapon.Shotgun] = true;
            this.weaponUnlocked[_ge.Weapon.RocketLauncher] = true;

            for (var ammoType = _ge.Consumable.AmmoTypes_Start + 1; ammoType < _ge.Consumable.AmmoTypes_End; ++ammoType) {
                this.ammoPouch.ammo[ammoType] = 1000;
            }
        }
    }

    Tick(): void {

        if (this.gunComp[_ge.Weapon.Pistol] == null) {
            this.gunComp[_ge.Weapon.Pistol] = this.guns[_ge.Weapon.Pistol].TryGetScriptComponent("Pistol");
            this.gunComp[_ge.Weapon.Shotgun] = this.guns[_ge.Weapon.Shotgun].TryGetScriptComponent("Shotgun");
            this.gunComp[_ge.Weapon.MachineGun] = this.guns[_ge.Weapon.MachineGun].TryGetScriptComponent("MachineGun");
            this.gunComp[_ge.Weapon.PlasmaRifle] = this.guns[_ge.Weapon.PlasmaRifle].TryGetScriptComponent("PlasmaRifle");
            this.gunComp[_ge.Weapon.RocketLauncher] = this.guns[_ge.Weapon.RocketLauncher].TryGetScriptComponent("RocketLauncher");

            this.SwitchToWeapon(_ge.Weapon.Pistol);
            return;
        }

        if (this.health > 0) {

            // character controller update
            {
                let msg = new wd.MsgMoveCharacterController();

                msg.Jump = this.input.GetCurrentInputState("Jump", true) > 0.5;
                msg.MoveForwards = this.input.GetCurrentInputState("MoveForwards", false);
                msg.MoveBackwards = this.input.GetCurrentInputState("MoveBackwards", false);
                msg.StrafeLeft = this.input.GetCurrentInputState("StrafeLeft", false);
                msg.StrafeRight = this.input.GetCurrentInputState("StrafeRight", false);
                msg.RotateLeft = this.input.GetCurrentInputState("RotateLeft", false);
                msg.RotateRight = this.input.GetCurrentInputState("RotateRight", false);
                msg.Run = this.input.GetCurrentInputState("Run", false) > 0.5;
                msg.Crouch = this.input.GetCurrentInputState("Crouch", false) > 0.5;

                this.characterController.SendMessage(msg);

                if (this.blackboard)
                {
                    // this is used to control the animation playback on the 'shadow proxy' mesh
                    // currently we only sync basic movement
                    // also note that the character mesh currently doesn't have crouch animations
                    // so we can't have a proper shadow there

                    this.blackboard.SetEntryValue("MoveForwards", msg.MoveForwards);
                    this.blackboard.SetEntryValue("MoveBackwards", msg.MoveBackwards);
                    this.blackboard.SetEntryValue("StrafeLeft", msg.StrafeLeft);
                    this.blackboard.SetEntryValue("StrafeRight", msg.StrafeRight);
                    this.blackboard.SetEntryValue("TouchingGround", this.characterController.IsStandingOnGround());
                }
            }

            // look up / down
            {
                let up = this.input.GetCurrentInputState("LookUp", false);
                let down = this.input.GetCurrentInputState("LookDown", false);

                this.headBone.ChangeVerticalRotation(down - up);
            }
        }

        wd.Debug.DrawInfoText(wd.Debug.ScreenPlacement.TopLeft, "Health: " + Math.ceil(this.health));

        if (this.activeWeapon != _ge.Weapon.None) {

            const ammoInClip = this.gunComp[this.activeWeapon].GetAmmoInClip();

            if (this.gunComp[this.activeWeapon].GetAmmoType() == _ge.Consumable.Ammo_None) {
                wd.Debug.DrawInfoText(wd.Debug.ScreenPlacement.TopLeft, "Ammo: " + ammoInClip);
            } else {
                const ammoOfType = this.ammoPouch.ammo[this.gunComp[this.activeWeapon].GetAmmoType()];
                wd.Debug.DrawInfoText(wd.Debug.ScreenPlacement.TopLeft, "Ammo: " + ammoInClip + " / " + ammoOfType);
            }

            this.gunComp[this.activeWeapon].RenderCrosshair();
        }
    }

    static RegisterMessageHandlers() {

        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgInputActionTriggered, "OnMsgInputActionTriggered");
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgDamage, "OnMsgMsgDamage");
        wd.TypescriptComponent.RegisterMessageHandler(_gm.MsgAddConsumable, "OnMsgAddConsumable");
        wd.TypescriptComponent.RegisterMessageHandler(_gm.MsgUnlockWeapon, "OnMsgUnlockWeapon");
        wd.TypescriptComponent.RegisterMessageHandler(wd.MsgPhysicsJointBroke, "OnMsgPhysicsJointBroke");
    }

    OnMsgPhysicsJointBroke(msg: wd.MsgPhysicsJointBroke): void {
        // must be the 'object grabber' joint

        this.SwitchToWeapon(this.holsteredWeapon);
    }

    OnMsgInputActionTriggered(msg: wd.MsgInputActionTriggered): void {

        if (this.health <= 0)
            return;

        if (msg.TriggerState == wd.TriggerState.Activated) {

            if (msg.InputAction == "Flashlight") {
                this.flashlight.SetActiveFlag(!this.flashlight.GetActiveFlag());
            }

            if (!this.grabObject.HasObjectGrabbed()) {

                if (msg.InputAction == "SwitchWeapon0")
                    this.SwitchToWeapon(_ge.Weapon.None);

                if (msg.InputAction == "SwitchWeapon1")
                    this.SwitchToWeapon(_ge.Weapon.Pistol);

                if (msg.InputAction == "SwitchWeapon2")
                    this.SwitchToWeapon(_ge.Weapon.Shotgun);

                if (msg.InputAction == "SwitchWeapon3")
                    this.SwitchToWeapon(_ge.Weapon.MachineGun);

                if (msg.InputAction == "SwitchWeapon4")
                    this.SwitchToWeapon(_ge.Weapon.PlasmaRifle);

                if (msg.InputAction == "SwitchWeapon5")
                    this.SwitchToWeapon(_ge.Weapon.RocketLauncher);

            }

            if (msg.InputAction == "Use") {

                if (this.grabObject.HasObjectGrabbed()) {
                    this.grabObject.DropGrabbedObject();
                    this.SwitchToWeapon(this.holsteredWeapon);
                }
                else if (this.grabObject.GrabNearbyObject()) {
                    this.holsteredWeapon = this.activeWeapon;
                    this.SwitchToWeapon(_ge.Weapon.None);
                }
                else {
                    
                    let hit = wd.Physics.Raycast(this.camera.GetGlobalPosition(), this.camera.GetGlobalDirForwards(), 2.0, 8);

                    if (hit != null && hit.actorObject) {

                        let msg = new wd.MsgGenericEvent;
                        msg.Message = "Use";

                        hit.actorObject.SendEventMessage(msg, this);
                    }
                }
            }

            if (msg.InputAction == "Teleport") {
                let owner = this.characterController.GetOwner();
                let pos = owner.GetGlobalPosition();
                let dir = owner.GetGlobalDirForwards();
                dir.z = 0;
                dir.Normalize();
                dir.MulNumber(5.0);
                pos.AddVec3(dir);

                // TODO:
                // if (this.characterController.IsDestinationUnobstructed(pos, 0)) {
                     this.characterController.TeleportCharacter(pos);
                // }
            }
        }

        if (msg.InputAction == "Shoot") {

            if (this.requireNoShoot) {
                if (msg.TriggerState == wd.TriggerState.Activated) {
                    this.requireNoShoot = false;
                }
            }

            if (!this.requireNoShoot) {

                if (this.grabObject.HasObjectGrabbed()) {
                    let dir = new wd.Vec3(0.75, 0, 0);
                    dir.MulNumber(30);
                    this.grabObject.ThrowGrabbedObject(dir);

                    this.SwitchToWeapon(this.holsteredWeapon);
                }
                else if (this.guns[this.activeWeapon]) {
                    let msgInteract = new _guns.MsgGunInteraction();
                    msgInteract.keyState = msg.TriggerState;
                    msgInteract.ammoPouch = this.ammoPouch;
                    msgInteract.interaction = _guns.GunInteraction.Fire;

                    this.guns[this.activeWeapon].SendMessage(msgInteract);
                }
            }
        }

        if (msg.InputAction == "Reload") {

            if (this.guns[this.activeWeapon]) {

                let msgInteract = new _guns.MsgGunInteraction();
                msgInteract.keyState = msg.TriggerState;
                msgInteract.ammoPouch = this.ammoPouch;
                msgInteract.interaction = _guns.GunInteraction.Reload;

                this.guns[this.activeWeapon].SendMessage(msgInteract);

            }
        }
    }

    health: number = 100;

    OnMsgMsgDamage(msg: wd.MsgDamage): void {

        if (this.Invincible)
            return;

        if (this.health <= 0)
            return;

        this.health -= msg.Damage * 2;

        if (this.health <= 0) {

            wd.Log.Info("Player died.");

            let owner = this.GetOwner();

            let camera = owner.FindChildByName("Camera");

            let camPos = camera.GetGlobalPosition();

            let go = new wd.GameObjectDesc();
            go.LocalPosition = camera.GetGlobalPosition();
            go.Dynamic = true;

            let rbCam = wd.World.CreateObject(go);

            let rbCamActor = wd.World.CreateComponent(rbCam, wd.JoltDynamicActorComponent);
            let rbCamSphere = wd.World.CreateComponent(rbCam, wd.JoltShapeSphereComponent);
            rbCamSphere.Radius = 0.3;
            let rbCamLight = wd.World.CreateComponent(rbCam, wd.PointLightComponent);
            rbCamLight.LightColor = wd.Color.DarkRed();
            rbCamLight.Intensity = 200;

            rbCamActor.Mass = 30;
            rbCamActor.LinearDamping = 0.5;
            rbCamActor.AngularDamping = 0.99;
            rbCamActor.CollisionLayer = 2; // debris
            rbCamActor.AddAngularForce(wd.Vec3.CreateRandomPointInSphere());

            camera.SetParent(rbCam);
        }
    }

    OnMsgAddConsumable(msg: _gm.MsgAddConsumable): void {

        const maxAmount = _ge.MaxConsumableAmount[msg.consumableType];

        if (msg.consumableType == _ge.Consumable.Health) {

            if (this.health <= 0 || this.health >= maxAmount) {
                msg.return_consumed = false;
                return;
            }

            msg.return_consumed = true;

            this.health = wd.Utils.Clamp(this.health + msg.amount, 1, 100);

            return;
        }

        if (msg.consumableType > _ge.Consumable.AmmoTypes_Start && msg.consumableType < _ge.Consumable.AmmoTypes_End) {

            const curAmount = this.ammoPouch.ammo[msg.consumableType]

            if (curAmount >= maxAmount) {
                msg.return_consumed = false
                return
            }

            msg.return_consumed = true

            const newAmount = curAmount + msg.amount;

            this.ammoPouch.ammo[msg.consumableType] = wd.Utils.Clamp(newAmount, 0, maxAmount);
        }
    }

    SwitchToWeapon(weapon: _ge.Weapon) {

        if (this.weaponUnlocked[weapon] == undefined || this.weaponUnlocked[weapon] == false)
            return;

        if (this.activeWeapon == weapon)
            return;

        this.requireNoShoot = true;

        if (this.gunComp[this.activeWeapon])
            this.gunComp[this.activeWeapon].DeselectGun();

        this.activeWeapon = weapon;

        if (this.gunComp[this.activeWeapon])
            this.gunComp[this.activeWeapon].SelectGun();
    }

    OnMsgUnlockWeapon(msg: _gm.MsgUnlockWeapon): void {

        msg.return_consumed = true;

        if (this.weaponUnlocked[msg.WeaponType] == undefined || this.weaponUnlocked[msg.WeaponType] == false) {

            this.weaponUnlocked[msg.WeaponType] = true;
            this.SwitchToWeapon(msg.WeaponType);
        }
    }
}