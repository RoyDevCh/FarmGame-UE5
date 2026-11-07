
import os, unreal
SRC = r"C:\Users\rjq51\Documents\dsh\UE5adaptation\_ref\FarmGame\Assets\FarmingEngine\Models"

FILES = []
for d in ["Constructions", "Other"]:
    base = os.path.join(SRC, d)
    if os.path.isdir(base):
        for f in os.listdir(base):
            if f.lower().endswith(".fbx"):
                FILES.append((d, f))
for f in ["Tree1.fbx","Tree2.fbx","Tree3.fbx","TreeDead.fbx","DirtRoad.fbx","StoneTile1.fbx","Pond.fbx","soil_pile.fbx","Cactus.fbx"]:
    FILES.append(("Environment", f))

tasks = []
for d, f in FILES:
    p = os.path.join(SRC, d, f)
    if not os.path.exists(p):
        continue
    name = os.path.splitext(f)[0].replace(" ", "_").replace("-", "_")
    t = unreal.AssetImportTask()
    t.filename = p
    t.destination_path = "/Game/Art/Props"
    t.destination_name = name
    t.automated = True
    t.replace_existing = True
    t.save = True
    o = unreal.FbxImportUI()
    o.set_editor_property("import_mesh", True)
    o.set_editor_property("import_materials", True)
    o.set_editor_property("import_textures", False)
    o.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
    o.static_mesh_import_data.set_editor_property("combine_meshes", True)
    t.options = o
    tasks.append(t)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
unreal.log("Props: imported %d" % len(tasks))
