
import unreal
unreal.EditorAssetLibrary.make_directory("/Game/Art/Materials")

def make_material(name, color):
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    mat = tools.create_asset(name, "/Game/Art/Materials", unreal.Material, unreal.MaterialFactoryNew())
    if not mat:
        unreal.log_warning("FarmMat: FAILED " + name)
        return None
    expr = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionConstant3Vector, -400, 0)
    expr.set_editor_property("constant", unreal.LinearColor(color[0], color[1], color[2], 1.0))
    unreal.MaterialEditingLibrary.connect_material_property(expr, "", unreal.MaterialProperty.MP_BASE_COLOR)
    rough = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionConstant, -400, 160)
    rough.set_editor_property("r", 0.85)
    unreal.MaterialEditingLibrary.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(mat.get_path_name())
    unreal.log("FarmMat: " + mat.get_path_name())
    return mat

make_material("M_FarmSoil", (0.24, 0.15, 0.09))
make_material("M_FarmGrass", (0.13, 0.28, 0.09))
make_material("M_FarmWood", (0.32, 0.20, 0.10))
