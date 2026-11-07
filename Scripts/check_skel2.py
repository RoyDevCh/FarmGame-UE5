
import unreal
for p in ["/Game/Art/Character/FarmerBoy", "/Game/Art/Animals/Chicken", "/Game/Art/Animals/Cow", "/Game/Art/Animals/bear"]:
    a = unreal.load_asset(p)
    if not a:
        print("RESULT", p, "MISSING")
        continue
    cls = a.get_class().get_name()
    print("RESULT", p, cls)
    if cls == "SkeletalMesh":
        sk = a.get_editor_property("skeleton")
        print("RESULT skeleton", sk.get_path_name() if sk else "none")
    print("RESULT props", [x for x in dir(unreal.FbxImportUI) if "mesh" in x.lower() or "skeletal" in x.lower()][:10])
