
import os, unreal
SRC = r"C:\Users\rjq51\Documents\dsh\UE5adaptation\_ref\FarmGame\Assets\FarmingEngine\Models"

def import_mesh(rel, dest, name):
    p = os.path.join(SRC, rel.replace("/", os.sep))
    if not os.path.exists(p):
        unreal.log_warning("Skel: missing " + p)
        return
    t = unreal.AssetImportTask()
    t.filename = p
    t.destination_path = dest
    t.destination_name = name
    t.automated = True
    t.replace_existing = True
    t.save = True
    o = unreal.FbxImportUI()
    o.set_editor_property("import_mesh", True)
    o.set_editor_property("import_materials", True)
    o.set_editor_property("import_textures", False)
    o.set_editor_property("import_animations", False)
    o.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    t.options = o
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])

import_mesh("Animals/Chicken.fbx", "/Game/Art/Animals", "ChickenSkel")
import_mesh("Animals/Cow.fbx", "/Game/Art/Animals", "CowSkel")
import_mesh("Animals/bear.fbx", "/Game/Art/Animals", "BearSkel")

def import_anim(rel, dest, name, skel_path):
    p = os.path.join(SRC, rel.replace("/", os.sep))
    if not os.path.exists(p):
        unreal.log_warning("Skel anim missing " + p)
        return
    skel = unreal.load_asset(skel_path)
    if not skel:
        unreal.log_warning("Skel missing " + skel_path)
        return
    t = unreal.AssetImportTask()
    t.filename = p
    t.destination_path = dest
    t.destination_name = name
    t.automated = True
    t.replace_existing = True
    t.save = True
    o = unreal.FbxImportUI()
    o.set_editor_property("import_mesh", False)
    o.set_editor_property("import_animations", True)
    o.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
    o.set_editor_property("skeleton", skel)
    t.options = o
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])

CH = "/Game/Art/Character/FarmerBoySkel_Skeleton"
import_anim("Character/Anims/character@idle.fbx", "/Game/Art/Character", "character_idle", CH)
import_anim("Character/Anims/character@run.fbx", "/Game/Art/Character", "character_run", CH)
import_anim("Character/Anims/character@attack.fbx", "/Game/Art/Character", "character_attack", CH)

CHICK = "/Game/Art/Animals/ChickenSkel_Skeleton"
import_anim("Animals/Chicken@idle.fbx", "/Game/Art/Animals", "chicken_idle", CHICK)
import_anim("Animals/Chicken@walk.fbx", "/Game/Art/Animals", "chicken_walk", CHICK)

COW = "/Game/Art/Animals/CowSkel_Skeleton"
import_anim("Animals/Cow@idle.fbx", "/Game/Art/Animals", "cow_idle", COW)
import_anim("Animals/Cow@walk.fbx", "/Game/Art/Animals", "cow_walk", COW)

BEAR = "/Game/Art/Animals/BearSkel_Skeleton"
import_anim("Animals/bear@idle.fbx", "/Game/Art/Animals", "bear_idle", BEAR)
import_anim("Animals/bear@run.fbx", "/Game/Art/Animals", "bear_run", BEAR)
import_anim("Animals/bear@attack.fbx", "/Game/Art/Animals", "bear_attack", BEAR)
unreal.log("Skel: done")
