
import os
import unreal

SRC = r"C:\Users\rjq51\Documents\dsh\UE5adaptation\_ref\FarmGame\Assets\FarmingEngine\Models"
ASSETS = [
    ("Character/FarmerBoy.fbx", "/Game/Art/Character"),
    ("Animals/Chicken.fbx", "/Game/Art/Animals"),
    ("Animals/Cow.fbx", "/Game/Art/Animals"),
    ("Animals/bear.fbx", "/Game/Art/Animals"),
    ("Farming/soil.fbx", "/Game/Art/Farming"),
    ("Farming/soil_hoed.fbx", "/Game/Art/Farming"),
    ("Environment/CarrotPlant.fbx", "/Game/Art/Farming"),
    ("Farming/BeanPlant.fbx", "/Game/Art/Farming"),
    ("Farming/Pepper plant.fbx", "/Game/Art/Farming"),
    ("Farming/strawberry_plant.fbx", "/Game/Art/Farming"),
    ("Farming/Short_Plant.fbx", "/Game/Art/Farming"),
    ("Constructions/fence_long.fbx", "/Game/Art/Buildings"),
    ("Constructions/furnace.fbx", "/Game/Art/Buildings"),
    ("Constructions/Chest.fbx", "/Game/Art/Buildings"),
    ("Environment/rock1.fbx", "/Game/Art/Environment"),
    ("Environment/Boulder.fbx", "/Game/Art/Environment"),
    ("Farming/TreeFruitL2.fbx", "/Game/Art/Environment"),
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
    task.destination_name = os.path.splitext(os.path.basename(path))[0]
    task.automated = True
    task.replace_existing = True
    task.save = True
    task.options = unreal.FbxImportUI()
    task.options.import_mesh = True
    task.options.import_materials = True
    task.options.import_textures = False
    task.options.static_mesh_import_data.combine_meshes = True
    task.options.static_mesh_import_data.generate_lightmap_u_vs = True
    tasks.append(task)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
unreal.log("FarmArt: imported %d tasks" % len(tasks))
