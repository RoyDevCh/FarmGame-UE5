
import unreal
for p in ["/Game/Art/Character/FarmerBoy", "/Game/Art/Character/FarmerBoy_Skeleton", "/Game/Art/Animals/Chicken", "/Game/Art/Animals/Chicken_Skeleton"]:
    a = unreal.load_asset(p)
    print(p, "->", a.get_class().get_name() if a else "MISSING")
    if a and a.get_class().get_name() == "SkeletalMesh":
        sk = a.get_editor_property("skeleton")
        print("   skeleton:", sk.get_path_name() if sk else None)
