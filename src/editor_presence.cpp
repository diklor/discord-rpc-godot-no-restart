#include "editor_presence.h"
#include "lib/discord_game_sdk/cpp/discord.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

EditorPresence *EditorPresence::singleton = nullptr;

void EditorPresence::_bind_methods()
{
}
EditorPresence::EditorPresence()
{
    singleton = this;
}
EditorPresence::~EditorPresence()
{
    singleton = nullptr;
    delete core; // couldn't use destructor because it would not compile on linux
    core = nullptr;
}
EditorPresence *EditorPresence::get_singleton()
{
    return singleton;
}

void EditorPresence::_ready()
{
    result = discord::Core::Create(1108142249990176808, DiscordCreateFlags_NoRequireDiscord, &core);
    activity.SetState("Editing a project...");
    activity.SetDetails(String(project_settings->get_setting("application/config/name")).utf8());
    if (project_settings->has_setting("application/config/name"))
    {
        activity.GetAssets().SetLargeImage("godot");
    }
    activity.GetAssets().SetLargeText(String(engine->get_version_info()["string"]).utf8());
    activity.GetTimestamps().SetStart(time->get_unix_time_from_system());
    if (result == discord::Result::Ok)
        core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
    else
        UtilityFunctions::push_warning("EditorPresence couldn't be loaded! Maybe your Discord isn't running?");
}

void EditorPresence::_process(double delta)
{
    godot::Node *edited_scene_root = get_tree()->get_edited_scene_root();

    if (edited_scene_root != nullptr)
    {
        godot::String scene_path = edited_scene_root->get_scene_file_path();

        if (scene_path.is_empty())
        {
            state_string = "Editing: (not saved scene)";
        }
        else
        {
            state_string = "Editing: \"" + scene_path.replace("res://", "") + "\"";
        }

        if (state_string.utf8() != activity.GetState())
        {
            activity.SetState(state_string.utf8());
            if (result == discord::Result::Ok)
                core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
        }
    }
    else
    {
        godot::String default_state = "No Scene Loaded";
        if (default_state.utf8() != activity.GetState())
        {
            activity.SetState(default_state.utf8());
            if (result == discord::Result::Ok)
                core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
        }
    }

    if (result == discord::Result::Ok)
        core->RunCallbacks();
}