
import os, unreal

SRC = r"C:\Users\rjq51\Documents\dsh\UE5adaptation\_ref\FarmGame\Assets\FarmingEngine\Models"

MESHES = [
    ("Character/FarmerBoy.fbx", "/Game/Art/Character", "FarmerBoy"),
    ("Animals/Chicken.fbx", "/Game/Art/Animals", "Chicken"),
    ("Animals/Cow.fbx", "/Game/Art/Animals", "Cow"),
    ("Animals/bear.fbx", "/Game/Art/Animals", "bear"),
]

def fbx(rel):
    return os.path.join(SRC, rel.replace("/", os.sep))

def make_task(path, dest, name, skeletal):
    t = unreal.AssetImportTask()
    t.filename = path
    t.destination_path = dest
    t.destination_name = name
    t.automated = True
    t.replace_existing = True
    t.save = True
    o = unreal.FbxImportUI()
    o.import_mesh = True
    o.import_materials = True
    o.import_textures = False
    o.import_animations = False
    o.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH if skeletal else unreal.FBXImportType.FBXIT_STATIC_MESH
    t.options = o
    return t

tasks = []
for rel, dest, name in MESHES:
    p = fbx(rel)
    if os.path.exists(p):
        tasks.append(make_task(p, dest, name, True))
    else:
        unreal.log_warning("FarmAnim: missing mesh " + p)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

SKELETONS = {
    "/Game/Art/Character": "FarmerBoy_Skeleton",
    "/Game/Art/Animals": ["Chicken_Skeleton", "Cow_Skeleton", "bear_Skeleton"],
}

ANIMS = [
    ("Character/character@idle.fbx", "/Game/Art/Character", "character_idle", "/Game/Art/Character/FarmerBoy_Skeleton"),
    ("Character/character@run.fbx", "/Game/Art/Character", "character_run", "/Game/Art/Character/FarmerBoy_Skeleton"),
    ("Character/character@attack.fbx", "/Game/Art/Character", "character_attack", "/Game/Art/Character/FarmerBoy_Skeleton"),
    ("Animals/Chicken@idle.fbx", "/Game/Art/Animals", "chicken_idle", "/Game/Art/Animals/Chicken_Skeleton"),
    ("Animals/Chicken@walk.fbx", "/Game/Art/Animals", "chicken_walk", "/Game/Art/Animals/Chicken_Skeleton"),
    ("Animals/Cow@idle.fbx", "/Game/Art/Animals", "cow_idle", "/Game/Art/Animals/Cow_Skeleton"),
    ("Animals/Cow@walk.fbx", "/Game/Art/Animals", "cow_walk", "/Game/Art/Animals/Cow_Skeleton"),
    ("Animals/bear@idle.fbx", "/Game/Art/Animals", "bear_idle", "/Game/Art/Animals/bear_Skeleton"),
    ("Animals/bear@run.fbx", "/Game/Art/Animals", "bear_run", "/Game/Art/Animals/bear_Skeleton"),
    ("Animals/bear@attack.fbx", "/Game/Art/Animals", "bear_attack", "/Game/Art/Animals/bear_Skeleton"),
]

anim_tasks = []
for rel, dest, name, skel_path in ANIMS:
    p = fbx(rel)
    if not os.path.exists(p):
        unreal.log_warning("FarmAnim: missing anim " + p)
        continue
    skel = unreal.load_asset(skel_path)
    if not skel:
        unreal.log_warning("FarmAnim: missing skeleton " + skel_path)
        continue
    t = unreal.AssetImportTask()
    t.filename = p
    t.destination_path = dest
    t.destination_name = name
    t.automated = True
    t.replace_existing = True
    t.save = True
    o = unreal.FbxImportUI()
    o.import_mesh = False
    o.import_animations = True
    o.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
    o.skeleton = skel
    o.anim_sequence_import_data.set_editor_property("import_bone_tracks", True)
    t.options = o
    anim_tasks.append(t)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(anim_tasks)
unreal.log("FarmAnim: meshes %d, anims %d" % (len(tasks), len(anim_tasks)))
