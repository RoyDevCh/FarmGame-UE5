
import unreal
for p in ["/Game/Art/Character/FarmerBoy", "/Game/Art/Animals/Chicken", "/Game/Art/Animals/Cow", "/Game/Art/Animals/bear"]:
    a = unreal.load_asset(p)
    if not a:
        unreal.log("RESULT %s MISSING" % p)
        continue
    cls = a.get_class().get_name()
    unreal.log("RESULT %s %s" % (p, cls))
    if cls == "SkeletalMesh":
        sk = a.get_editor_property("skeleton")
        unreal.log("RESULT skeleton %s" % (sk.get_path_name() if sk else "none"))
unreal.log("RESULT fbximp %s" % str([x for x in dir(unreal) if "FBXImportType" in x]))
