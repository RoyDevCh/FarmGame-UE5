
import os, unreal
SRC = r"C:\Users\rjq51\Documents\dsh\UE5adaptation\FarmGameUE\Content\Art\Textures_Source"
unreal.EditorAssetLibrary.make_directory("/Game/Art/Textures")

tasks = []
for name in ["T_FarmGrass", "T_FarmDirt", "T_FarmWood"]:
    p = os.path.join(SRC, name + ".png")
    t = unreal.AssetImportTask()
    t.filename = p
    t.destination_path = "/Game/Art/Textures"
    t.destination_name = name
    t.automated = True
    t.replace_existing = True
    t.save = True
    tasks.append(t)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

def make_tiled(name, tex_path, tiling, rough):
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    try:
        unreal.EditorAssetLibrary.delete_asset("/Game/Art/Materials/" + name)
    except Exception:
        pass
    mat = tools.create_asset(name, "/Game/Art/Materials", unreal.Material, unreal.MaterialFactoryNew())
    if not mat:
        unreal.log_warning("MatTex: create failed " + name)
        return
    tex = unreal.load_asset(tex_path)
    ts = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -450, 0)
    ts.set_editor_property("texture", tex)
    try:
        uv = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -800, 0)
        uv.set_editor_property("u_tiling", tiling)
        uv.set_editor_property("v_tiling", tiling)
        unreal.MaterialEditingLibrary.connect_material_expressions(uv, "UVs", ts, "UVs")
    except Exception as e:
        unreal.log_warning("MatTex: uv tiling failed %s %s" % (name, e))
    unreal.MaterialEditingLibrary.connect_material_property(ts, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    r = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant, -450, 180)
    r.set_editor_property("r", rough)
    unreal.MaterialEditingLibrary.connect_material_property(r, "", unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(mat.get_path_name())
    unreal.log("MatTex: " + mat.get_path_name())

make_tiled("M_FarmGrassTiled", "/Game/Art/Textures/T_FarmGrass", 80.0, 0.95)
make_tiled("M_FarmSoilTiled", "/Game/Art/Textures/T_FarmDirt", 1.0, 0.9)
make_tiled("M_FarmWoodTiled", "/Game/Art/Textures/T_FarmWood", 2.0, 0.8)
