
import os, unreal
SRC = r"C:\Users\rjq51\Documents\dsh\UE5adaptation\_ref\FarmGame\Assets\FarmingEngine\Audio\SFX"
FILES = ["craft.wav", "Build.wav", "Hit-wood.wav", "Hit-Stone.wav", "Flesh Hit.wav", "Pop.wav", "fast_woosh.wav"]
tasks = []
for name in FILES:
    p = os.path.join(SRC, name)
    if not os.path.exists(p):
        unreal.log_warning("FarmAudio: missing " + p)
        continue
    t = unreal.AssetImportTask()
    t.filename = p
    t.destination_path = "/Game/Art/Audio"
    t.destination_name = os.path.splitext(name)[0].replace(" ", "_").replace("-", "_").lower()
    t.automated = True
    t.replace_existing = True
    t.save = True
    tasks.append(t)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
unreal.log("FarmAudio: imported %d" % len(tasks))
