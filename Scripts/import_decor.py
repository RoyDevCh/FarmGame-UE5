
import os
import unreal

SRC = r"C:\Users\rjq51\Documents\dsh\UE5adaptation\_ref\FarmGame\Assets\FarmingEngine\Models"
ASSETS = [
    ("Constructions/barn.fbx", "/Game/Art/Buildings"),
    ("Constructions/house.fbx", "/Game/Art/Buildings"),
    ("Constructions/Windmill.fbx", "/Game/Art/Buildings"),
    ("Constructions/wall_square.fbx", "/Game/Art/Buildings"),
    ("Constructions/fence_gate.fbx", "/Game/Art/Buildings"),
    ("Constructions/Chest stone.fbx", "/Game/Art/Buildings"),
    ("Environment/Palm Tree.fbx", "/Game/Art/Environment"),
    ("Environment/Pine cone.fbx", "/Game/Art/Environment"),
    ("Environment/Bush.fbx", "/Game/Art/Environment"),
    ("Environment/Flower.fbx", "/Game/Art/Environment"),
    ("Environment/metal rock.fbx", "/Game/Art/Environment"),
    ("Environment/rock2.fbx", "/Game/Art/Environment"),
]

tasks = []
for rel, dest in ASSETS:
    path = os.path.join(SRC, rel.replace("/", os.sep))
    if not os.path.exists(path):
        unreal.log_warning("FarmArt: missing " + path)
        continue
    task = unreal.AssetImportTask()
    task.filename = path
    task.destination_path = dest
    task.destination_name = os.path.splitext(os.path.basename(path))[0].replace(" ", "_")
    task.automated = True
    task.replace_existing = True
    task.save = True
    task.options = unreal.FbxImportUI()
    task.options.import_mesh = True
    task.options.import_materials = True
    task.options.import_textures = False
    task.options.static_mesh_import_data.combine_meshes = True
    tasks.append(task)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
unreal.log("FarmArt: decor import tasks %d" % len(tasks))
