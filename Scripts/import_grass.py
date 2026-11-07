
import os, unreal
SRC = r"C:\Users\rjq51\Documents\dsh\UE5adaptation\_ref\FarmGame\Assets\FarmingEngine\Models"
path = os.path.join(SRC, "Environment", "grass.fbx")
task = unreal.AssetImportTask()
task.filename = path
task.destination_path = "/Game/Art/Environment"
task.destination_name = "grass"
task.automated = True
task.replace_existing = True
task.save = True
task.options = unreal.FbxImportUI()
task.options.import_mesh = True
task.options.import_materials = True
task.options.import_textures = False
task.options.static_mesh_import_data.combine_meshes = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
unreal.log("FarmArt: grass imported")
