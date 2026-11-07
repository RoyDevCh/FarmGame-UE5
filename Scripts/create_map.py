import unreal

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
les.new_level("/Game/Maps/Farm")
les.save_current_level()

world = unreal.EditorLevelLibrary.get_editor_world()
unreal.log("FarmGame: created map /Game/Maps/Farm")
