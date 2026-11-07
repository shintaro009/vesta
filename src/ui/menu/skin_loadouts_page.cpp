#include <stdafx.hpp>
#include <imgui.h>
#include <windows.h>
#include <commdlg.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

namespace ui
{
namespace
{
struct entry { std::string category, name; float wear{0.05f}; bool stattrak{}; };
std::vector<entry> items;
int category{};
char item_name[128]{};
float item_wear{0.05f};
bool item_stattrak{};
int preset_index{};
std::string message{"Loadouts are local profiles only; they do not change items in CS2."};

struct preset { const char* name; std::vector<entry> entries; };
const std::vector<preset>& presets()
{
    static const std::vector<preset> data{
        {"s1mple-inspired", {{"Rifle","AK-47 | Case Hardened",0.08f,false},{"Sniper","AWP | Dragon Lore",0.18f,false},{"Pistol","Desert Eagle | Blaze",0.03f,false},{"Knife","Karambit | Doppler",0.02f,false},{"Gloves","Sport Gloves | Pandora's Box",0.22f,false}}},
        {"ZywOo-inspired", {{"Rifle","AK-47 | Bloodsport",0.11f,false},{"Sniper","AWP | Asiimov",0.19f,false},{"Pistol","Glock-18 | Fade",0.02f,false},{"Knife","Butterfly Knife | Slaughter",0.08f,false},{"Gloves","Sport Gloves | Vice",0.20f,false}}},
        {"donk-inspired", {{"Rifle","AK-47 | Neon Rider",0.09f,false},{"Rifle","M4A1-S | Decimator",0.13f,false},{"Sniper","AWP | Neo-Noir",0.10f,false},{"Knife","Butterfly Knife | Doppler",0.03f,false}}},
        {"m0NESY-inspired", {{"Rifle","AK-47 | Inheritance",0.10f,false},{"Sniper","AWP | Gungnir",0.07f,false},{"Pistol","USP-S | Orion",0.09f,false},{"Knife","Karambit | Marble Fade",0.03f,false}}},
        {"NiKo-inspired", {{"Rifle","AK-47 | Fire Serpent",0.18f,false},{"Sniper","AWP | Medusa",0.20f,false},{"Pistol","Desert Eagle | Blaze",0.02f,false},{"Knife","Butterfly Knife | Fade",0.01f,false}}},
        {"ropz-inspired", {{"Rifle","AK-47 | Vulcan",0.10f,false},{"Sniper","AWP | Lightning Strike",0.02f,false},{"Pistol","USP-S | Neo-Noir",0.11f,false},{"Knife","M9 Bayonet | Doppler",0.03f,false}}}
    };
    return data;
}
void save_profile()
{
    wchar_t path[MAX_PATH] = L"vesta-loadout.vsl";
    OPENFILENAMEW ofn{}; ofn.lStructSize=sizeof(ofn); ofn.lpstrFile=path; ofn.nMaxFile=MAX_PATH;
    ofn.lpstrFilter=L"Vesta Loadout (*.vsl)\0*.vsl\0All files (*.*)\0*.*\0";
    ofn.lpstrDefExt=L"vsl"; ofn.Flags=OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST;
    if (!GetSaveFileNameW(&ofn)) return;
    std::ofstream f(path, std::ios::binary);
    if (!f) { message="Could not save profile."; return; }
    f << "VESTA_LOADOUT_V1\n";
    for (const auto& x:items) f << x.category << "|" << x.name << "|" << x.wear << "|" << (x.stattrak?1:0) << "\n";
    message="Profile saved.";
}
void load_profile()
{
    wchar_t path[MAX_PATH]{};
    OPENFILENAMEW ofn{}; ofn.lStructSize=sizeof(ofn); ofn.lpstrFile=path; ofn.nMaxFile=MAX_PATH;
    ofn.lpstrFilter=L"Vesta Loadout (*.vsl)\0*.vsl\0All files (*.*)\0*.*\0";
    ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameW(&ofn)) return;
    std::ifstream f(path, std::ios::binary);
    std::string line;
    if (!std::getline(f,line) || line!="VESTA_LOADOUT_V1") { message="Unsupported profile format."; return; }
    std::vector<entry> loaded;
    while (std::getline(f,line)) {
        std::stringstream ss(line); entry x; int st{};
        if (std::getline(ss,x.category,'|') && std::getline(ss,x.name,'|') && (ss >> x.wear) && (ss.ignore(), ss >> st) && x.wear>=0.0f && x.wear<=1.0f)
            { x.stattrak=st!=0; loaded.push_back(std::move(x)); }
    }
    items=std::move(loaded); message="Profile loaded.";
}
}
void draw_skin_loadouts_page()
{
    ImGui::TextWrapped("Build and save cosmetic loadout ideas. This page does not connect to CS2 or apply skins in-game.");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("PLAYER-INSPIRED PRESETS");
    if (ImGui::BeginCombo("Preset", presets()[std::clamp(preset_index,0,(int)presets().size()-1)].name)) {
        for (int i=0;i<(int)presets().size();++i) {
            bool selected=preset_index==i;
            if (ImGui::Selectable(presets()[i].name,selected)) preset_index=i;
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Apply preset")) { items=presets()[preset_index].entries; message="Preset loaded (fan-made style, not a verified player inventory)."; }
    ImGui::Spacing();
    ImGui::Text("ADD ITEM");
    static const char* categories[]{"Rifle","Pistol","SMG","Shotgun","Sniper","Machine gun","Knife","Gloves","Other"};
    ImGui::SetNextItemWidth(150);
    ImGui::Combo("Category",&category,categories,(int)std::size(categories));
    ImGui::SetNextItemWidth(260);
    ImGui::InputText("Skin / item name",item_name,sizeof(item_name));
    ImGui::SetNextItemWidth(180);
    ImGui::SliderFloat("Wear / Float",&item_wear,0.0f,1.0f,"%.4f");
    ImGui::SameLine(); ImGui::Checkbox("StatTrak",&item_stattrak);
    if (ImGui::Button("Add item") && item_name[0]) {
        items.push_back({categories[category],item_name,item_wear,item_stattrak});
        item_name[0]='\0'; item_stattrak=false; message="Item added.";
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("CURRENT LOADOUT (%d)",(int)items.size());
    if (ImGui::BeginChild("##skin_loadout_entries",ImVec2(0,300),true)) {
        int remove=-1;
        for (int i=0;i<(int)items.size();++i) {
            ImGui::PushID(i);
            auto& x=items[i];
            ImGui::Text("[%s] %s  | wear %.4f%s",x.category.c_str(),x.name.c_str(),x.wear,x.stattrak?" | StatTrak":"");
            ImGui::SameLine();
            if (ImGui::SmallButton("Remove")) remove=i;
            ImGui::PopID();
        }
        if (remove>=0) items.erase(items.begin()+remove);
    }
    ImGui::EndChild();
    if (ImGui::Button("Save profile")) save_profile();
    ImGui::SameLine();
    if (ImGui::Button("Load profile")) load_profile();
    ImGui::SameLine();
    if (ImGui::Button("Clear all")) { items.clear(); message="Loadout cleared."; }
    ImGui::Spacing();
    ImGui::TextDisabled("%s",message.c_str());
}
} // namespace ui
