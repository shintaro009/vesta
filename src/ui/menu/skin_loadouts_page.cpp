#include <stdafx.hpp>
#include <imgui.h>
#include <windows.h>
#include <commdlg.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <array>

namespace ui
{
namespace
{
struct entry
{
    std::string category, name;
    float wear{0.05f};
    int seed{0};
    bool stattrak{};
};
std::vector<entry> items;
int category{};
char item_name[128]{};
char search_text[128]{};
float item_wear{0.05f};
int item_seed{};
bool item_stattrak{};
int preset_index{};
int selected_item{-1};
std::string message{"LOCAL COLLECTION  /  Profiles stay on this PC."};

constexpr const char* categories[]{"All", "Rifle", "Pistol", "SMG", "Shotgun", "Sniper", "Machine gun", "Knife", "Gloves", "Agent", "Other"};
constexpr ImVec4 accent{0.48f, 0.34f, 0.94f, 1.0f};
constexpr ImVec4 muted{0.55f, 0.57f, 0.64f, 1.0f};

struct preset { const char* name; const char* subtitle; ImVec4 tint; std::vector<entry> entries; };
const std::vector<preset>& presets()
{
    static const std::vector<preset> data{
        {"s1mple-inspired", "PRECISION / CLASSIC", {0.32f,0.48f,0.88f,1}, {{"Rifle","AK-47 | Case Hardened",0.08f,321,false},{"Sniper","AWP | Dragon Lore",0.18f,412,false},{"Pistol","Desert Eagle | Blaze",0.03f,0,false},{"Knife","Karambit | Doppler",0.02f,0,false},{"Gloves","Sport Gloves | Pandora's Box",0.22f,0,false}}},
        {"ZywOo-inspired", "VERSATILE / CLEAN", {0.36f,0.68f,0.58f,1}, {{"Rifle","AK-47 | Bloodsport",0.11f,0,false},{"Sniper","AWP | Asiimov",0.19f,0,false},{"Pistol","Glock-18 | Fade",0.02f,0,false},{"Knife","Butterfly Knife | Slaughter",0.08f,0,false},{"Gloves","Sport Gloves | Vice",0.20f,0,false}}},
        {"donk-inspired", "AGGRESSIVE / BOLD", {0.88f,0.40f,0.30f,1}, {{"Rifle","AK-47 | Neon Rider",0.09f,0,false},{"Rifle","M4A1-S | Decimator",0.13f,0,false},{"Sniper","AWP | Neo-Noir",0.10f,0,false},{"Knife","Butterfly Knife | Doppler",0.03f,0,false}}},
        {"m0NESY-inspired", "SNIPER / MODERN", {0.60f,0.42f,0.88f,1}, {{"Rifle","AK-47 | Inheritance",0.10f,0,false},{"Sniper","AWP | Gungnir",0.07f,0,false},{"Pistol","USP-S | Orion",0.09f,0,false},{"Knife","Karambit | Marble Fade",0.03f,0,false}}},
        {"NiKo-inspired", "RIFLE / HERITAGE", {0.84f,0.62f,0.28f,1}, {{"Rifle","AK-47 | Fire Serpent",0.18f,0,false},{"Sniper","AWP | Medusa",0.20f,0,false},{"Pistol","Desert Eagle | Blaze",0.02f,0,false},{"Knife","Butterfly Knife | Fade",0.01f,0,false}}},
        {"ropz-inspired", "MINIMAL / SHARP", {0.32f,0.70f,0.78f,1}, {{"Rifle","AK-47 | Vulcan",0.10f,0,false},{"Sniper","AWP | Lightning Strike",0.02f,0,false},{"Pistol","USP-S | Neo-Noir",0.11f,0,false},{"Knife","M9 Bayonet | Doppler",0.03f,0,false}}}
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
    f << "VESTA_LOADOUT_V2\n";
    for (const auto& x:items) f << x.category << "|" << x.name << "|" << x.wear << "|" << x.seed << "|" << (x.stattrak?1:0) << "\n";
    message="Profile exported successfully.";
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
    if (!std::getline(f,line) || (line!="VESTA_LOADOUT_V1" && line!="VESTA_LOADOUT_V2")) { message="Unsupported profile format."; return; }
    const bool v2=line=="VESTA_LOADOUT_V2";
    std::vector<entry> loaded;
    while (std::getline(f,line)) {
        std::stringstream ss(line); entry x; int st{};
        if (!std::getline(ss,x.category,'|') || !std::getline(ss,x.name,'|') || !(ss >> x.wear) || x.wear<0.0f || x.wear>1.0f) continue;
        if (v2) { if (!(ss.ignore(),ss>>x.seed) || x.seed<0 || x.seed>1000 || !(ss.ignore(),ss>>st)) continue; }
        else if (!(ss.ignore(),ss>>st)) continue;
        x.stattrak=st!=0; loaded.push_back(std::move(x));
    }
    items=std::move(loaded); selected_item=items.empty()?-1:0; message="Profile imported.";
}
ImU32 color(const ImVec4& c) { return ImGui::ColorConvertFloat4ToU32(c); }
void section_title(const char* eyebrow, const char* title)
{
    ImGui::TextColored(accent,"%s",eyebrow);
    ImGui::SameLine(0,10);
    ImGui::Text("%s",title);
}
void item_art(const entry& x, ImVec2 size, int id)
{
    ImDrawList* draw=ImGui::GetWindowDrawList();
    ImVec2 p=ImGui::GetCursorScreenPos();
    const ImVec4 tints[]={{0.30f,0.42f,0.78f,1},{0.70f,0.35f,0.38f,1},{0.34f,0.62f,0.58f,1},{0.65f,0.46f,0.78f,1},{0.78f,0.58f,0.30f,1}};
    const ImVec4 tint=tints[static_cast<unsigned>(id)%5];
    draw->AddRectFilled(p,{p.x+size.x,p.y+size.y},IM_COL32(23,25,35,255),9);
    draw->AddRectFilledMultiColor(p,{p.x+size.x,p.y+size.y},color({tint.x,tint.y,tint.z,0.52f}),IM_COL32(25,27,38,255),IM_COL32(18,20,29,255),color({tint.x,tint.y,tint.z,0.12f}));
    const float cy=p.y+size.y*0.52f;
    draw->AddLine({p.x+size.x*0.20f,cy},{p.x+size.x*0.78f,cy-3},IM_COL32(226,224,239,235),5.0f);
    draw->AddLine({p.x+size.x*0.36f,cy+1},{p.x+size.x*0.31f,cy+size.y*0.25f},IM_COL32(190,190,208,230),4.0f);
    draw->AddCircleFilled({p.x+size.x*0.77f,cy-3},3.0f,IM_COL32(244,205,130,255));
    draw->AddText({p.x+10,p.y+8},IM_COL32(235,235,245,190),x.category.c_str());
    ImGui::Dummy(size);
}
}
void draw_skin_loadouts_page()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,7.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{14,12});
    ImGui::TextColored(accent,"VESTA  /  COLLECTION STUDIO");
    ImGui::SameLine();
    ImGui::TextDisabled("LOCAL LOADOUT BUILDER");
    ImGui::Spacing();
    ImGui::TextWrapped("Curate your favourite finishes, organize themed kits and keep portable profiles.");
    ImGui::TextDisabled("Offline collection only — does not connect to CS2 or alter in-game items.");
    ImGui::Spacing();
    ImGui::Separator();

    section_title("01", "PLAYER-INSPIRED COLLECTIONS");
    ImGui::Spacing();
    const float card_w=ImGui::GetContentRegionAvail().x/3.0f-8.0f;
    for (int i=0;i<(int)presets().size();++i) {
        if (i%3) ImGui::SameLine(0,10);
        ImGui::PushID(i);
        ImGui::BeginChild("##preset_card",{card_w,84},true);
        ImGui::TextColored(presets()[i].tint,"%s",presets()[i].subtitle);
        ImGui::Text("%s",presets()[i].name);
        ImGui::TextDisabled("%d items",(int)presets()[i].entries.size());
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) { preset_index=i; items=presets()[i].entries; selected_item=items.empty()?-1:0; message="Preset loaded. Fan-made theme, not a verified player inventory."; }
        ImGui::EndChild();
        ImGui::PopID();
    }
    ImGui::Spacing();
    ImGui::Separator();
    section_title("02","YOUR COLLECTION");
    ImGui::Spacing();
    ImGui::SetNextItemWidth(210);
    ImGui::InputTextWithHint("##skin_search","Search item names...",search_text,sizeof(search_text));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(145);
    ImGui::Combo("##skin_category",&category,categories,(int)std::size(categories));
    ImGui::SameLine();
    ImGui::TextDisabled("%d ITEMS",(int)items.size());
    ImGui::Spacing();

    const float panel_w=ImGui::GetContentRegionAvail().x;
    const float detail_w=std::min(250.0f,panel_w*0.38f);
    ImGui::BeginChild("##collection_grid",{panel_w-detail_w-10,286},true);
    int remove=-1;
    int visible=0;
    for (int i=0;i<(int)items.size();++i) {
        auto& x=items[i];
        if (category>0 && x.category!=categories[category]) continue;
        if (search_text[0] && x.name.find(search_text)==std::string::npos) continue;
        if (visible && visible%2) ImGui::SameLine(0,8);
        ImGui::PushID(i);
        ImGui::BeginGroup();
        const bool active=selected_item==i;
        if (ImGui::Selectable("##item_card",active,ImGuiSelectableFlags_AllowDoubleClick,{(ImGui::GetContentRegionAvail().x-8)*0.5f,105})) selected_item=i;
        ImGui::SetCursorPosY(ImGui::GetCursorPosY()-ImGui::GetFrameHeight());
        item_art(x,{(ImGui::GetContentRegionAvail().x-8)*0.5f,55},i);
        ImGui::TextWrapped("%s",x.name.c_str());
        ImGui::TextDisabled("%s  /  %.4f",x.category.c_str(),x.wear);
        ImGui::EndGroup();
        ImGui::PopID();
        ++visible;
    }
    if (!visible) ImGui::TextDisabled("No matching items. Add one below or change your filters.");
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("##item_details",{0,286},true);
    ImGui::TextColored(accent,"SELECTED ITEM");
    if (selected_item>=0 && selected_item<(int)items.size()) {
        auto& x=items[selected_item];
        ImGui::TextWrapped("%s",x.name.c_str());
        ImGui::TextDisabled("%s",x.category.c_str());
        ImGui::Spacing();
        ImGui::SliderFloat("Float",&x.wear,0.0f,1.0f,"%.4f");
        ImGui::SliderInt("Pattern seed",&x.seed,0,1000);
        ImGui::Checkbox("StatTrak", &x.stattrak);
        ImGui::Spacing();
        if (ImGui::Button("Remove item")) { remove=selected_item; }
    } else ImGui::TextDisabled("Choose an item card to edit its properties.");
    ImGui::EndChild();
    if (remove>=0) { items.erase(items.begin()+remove); selected_item=items.empty()?-1:std::min(remove,(int)items.size()-1); message="Item removed."; }

    ImGui::Spacing();
    section_title("03","ADD TO COLLECTION");
    ImGui::SetNextItemWidth(130);
    ImGui::Combo("##new_category",&category,categories+1,(int)std::size(categories)-1);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(230);
    ImGui::InputTextWithHint("##new_item","Weapon | Finish",item_name,sizeof(item_name));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(110);
    ImGui::SliderFloat("##new_float",&item_wear,0.0f,1.0f,"%.3f");
    ImGui::SameLine();
    if (ImGui::Button("Add item") && item_name[0]) {
        items.push_back({categories[category+1],item_name,item_wear,item_seed,item_stattrak});
        selected_item=(int)items.size()-1; item_name[0]='\0'; item_stattrak=false; message="Item added to local collection.";
    }
    ImGui::Spacing();
    if (ImGui::Button("Export .vsl")) save_profile();
    ImGui::SameLine();
    if (ImGui::Button("Import .vsl")) load_profile();
    ImGui::SameLine();
    if (ImGui::Button("Clear collection")) { items.clear(); selected_item=-1; message="Collection cleared."; }
    ImGui::SameLine();
    ImGui::TextDisabled("%s",message.c_str());
    ImGui::PopStyleVar(3);
}
} // namespace ui
