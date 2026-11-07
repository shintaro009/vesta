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
#include <cmath>

namespace ui
{
namespace
{
struct entry
{
    std::string category, name;
    float wear{0.05f};
    int seed{};
    bool stattrak{};
};
std::vector<entry> items{
    {"Rifle","AK-47 | Neon Rider",0.09f,321,false},
    {"Rifle","M4A1-S | Printstream",0.08f,14,true},
    {"Sniper","AWP | Asiimov",0.19f,412,false},
    {"Pistol","Desert Eagle | Blaze",0.03f,0,false},
    {"Pistol","USP-S | Kill Confirmed",0.12f,88,true},
    {"Knife","Karambit | Doppler",0.02f,0,false},
    {"Gloves","Sport Gloves | Vice",0.20f,0,false}
};
int category{};
char search_text[128]{};
char item_name[128]{};
float item_wear{0.05f};
int item_seed{};
bool item_stattrak{};
int selected_item{};
int selected_weapon{};
int selected_finish{};
int preview_mode{};
std::string message{"Ready  /  Local profile"};
constexpr const char* categories[]{"All","Rifle","Pistol","SMG","Shotgun","Sniper","Machine gun","Knife","Gloves","Agent","Other"};
constexpr const char* weapons[]{"AK-47","M4A4","M4A1-S","AWP","Desert Eagle","USP-S","Glock-18","Karambit","Butterfly Knife","Gloves"};
constexpr const char* finishes[]{"Neon Rider","Redline","Case Hardened","Bloodsport","Asiimov","Printstream","Doppler","Fade","Slaughter","Dragon Lore"};
constexpr ImVec4 accent{0.55f,0.36f,0.98f,1.0f};
constexpr ImVec4 cyan{0.28f,0.78f,0.88f,1.0f};
constexpr ImVec4 muted{0.55f,0.57f,0.64f,1.0f};
ImU32 col(ImVec4 c){return ImGui::ColorConvertFloat4ToU32(c);}
void save_profile()
{
    wchar_t path[MAX_PATH]=L"vesta-skins.vsl";
    OPENFILENAMEW ofn{}; ofn.lStructSize=sizeof(ofn); ofn.lpstrFile=path; ofn.nMaxFile=MAX_PATH;
    ofn.lpstrFilter=L"Vesta Skin Profile (*.vsl)\0*.vsl\0All files (*.*)\0*.*\0";
    ofn.lpstrDefExt=L"vsl"; ofn.Flags=OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST;
    if(!GetSaveFileNameW(&ofn))return;
    std::ofstream f(path,std::ios::binary);
    if(!f){message="Could not save profile";return;}
    f<<"VESTA_LOADOUT_V2\n";
    for(const auto& x:items)f<<x.category<<"|"<<x.name<<"|"<<x.wear<<"|"<<x.seed<<"|"<<(x.stattrak?1:0)<<"\n";
    message="Profile exported";
}
void load_profile()
{
    wchar_t path[MAX_PATH]{};
    OPENFILENAMEW ofn{}; ofn.lStructSize=sizeof(ofn); ofn.lpstrFile=path; ofn.nMaxFile=MAX_PATH;
    ofn.lpstrFilter=L"Vesta Skin Profile (*.vsl)\0*.vsl\0All files (*.*)\0*.*\0";
    ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
    if(!GetOpenFileNameW(&ofn))return;
    std::ifstream f(path,std::ios::binary); std::string line;
    if(!std::getline(f,line)||(line!="VESTA_LOADOUT_V1"&&line!="VESTA_LOADOUT_V2")){message="Unsupported profile";return;}
    const bool v2=line=="VESTA_LOADOUT_V2"; std::vector<entry> loaded;
    while(std::getline(f,line)){
        std::stringstream ss(line); entry x; int st{};
        if(!std::getline(ss,x.category,'|')||!std::getline(ss,x.name,'|')||!(ss>>x.wear)||x.wear<0||x.wear>1)continue;
        if(v2){if(!(ss.ignore(),ss>>x.seed)||x.seed<0||x.seed>1000||!(ss.ignore(),ss>>st))continue;}
        else if(!(ss.ignore(),ss>>st))continue;
        x.stattrak=st!=0; loaded.push_back(std::move(x));
    }
    items=std::move(loaded); selected_item=items.empty()?-1:0; message="Profile imported";
}
void panel(const char* id,ImVec2 size)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg,ImVec4{0.075f,0.078f,0.105f,0.94f});
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,10.0f);
    ImGui::BeginChild(id,size,true,ImGuiWindowFlags_NoScrollbar);
}
void end_panel(){ImGui::EndChild();ImGui::PopStyleVar();ImGui::PopStyleColor();}
void section(const char* index,const char* title)
{
    ImGui::TextColored(accent,"%s",index); ImGui::SameLine(0,8);
    ImGui::Text("%s",title);
}
void weapon_art(ImVec2 pos,ImVec2 size,int variant)
{
    auto* d=ImGui::GetWindowDrawList();
    const ImVec4 tints[]={{0.45f,0.29f,0.90f,1},{0.16f,0.62f,0.75f,1},{0.78f,0.36f,0.36f,1},{0.78f,0.58f,0.25f,1}};
    const auto t=tints[variant%4];
    d->AddRectFilled(pos,{pos.x+size.x,pos.y+size.y},IM_COL32(23,25,36,255),9);
    d->AddRectFilledMultiColor(pos,{pos.x+size.x,pos.y+size.y},col({t.x,t.y,t.z,0.42f}),IM_COL32(25,27,38,255),IM_COL32(18,20,29,255),col({t.x,t.y,t.z,0.10f}));
    const float y=pos.y+size.y*0.53f;
    d->AddLine({pos.x+size.x*.16f,y},{pos.x+size.x*.80f,y-3},IM_COL32(230,230,241,245),6);
    d->AddLine({pos.x+size.x*.34f,y+1},{pos.x+size.x*.29f,y+size.y*.26f},IM_COL32(180,183,200,235),5);
    d->AddLine({pos.x+size.x*.70f,y-2},{pos.x+size.x*.77f,y-size.y*.18f},IM_COL32(190,192,210,230),3);
    d->AddCircleFilled({pos.x+size.x*.80f,y-3},3,IM_COL32(245,210,142,255));
}
}
void draw_skin_loadouts_page()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,7.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,{10,7});
    ImGui::SetCursorPos({16,14});
    ImGui::TextColored(accent,"VESTA  /  SKIN STUDIO");
    ImGui::SameLine(0,12); ImGui::TextDisabled("LOADOUT WORKSPACE");
    ImGui::SameLine(ImGui::GetContentRegionAvail().x-90);
    ImGui::TextColored(cyan,"●  READY");
    ImGui::SetCursorPosX(16);
    ImGui::TextDisabled("Build a collection, tune finish parameters and manage local presets.");
    ImGui::SetCursorPosX(16);
    ImGui::Spacing();

    const float full=ImGui::GetContentRegionAvail().x-32;
    const float gap=9.0f, statw=(full-gap*3)/4;
    const char* stats[][2]={{"COLLECTION",nullptr},{"WEAPONS","10"},{"FINISHES","10"},{"PROFILES","06"}};
    const std::string count=std::to_string(items.size());
    stats[0][1]=count.c_str();
    for(int i=0;i<4;++i){
        if(i)ImGui::SameLine(0,gap);
        panel(("##stat"+std::to_string(i)).c_str(),{statw,52});
        ImGui::TextColored(muted,"%s",stats[i][0]); ImGui::SameLine();
        ImGui::TextColored(i==0?accent:ImVec4{0.92f,0.92f,0.96f,1},"%s",stats[i][1]);
        end_panel();
    }
    ImGui::Spacing();
    const float leftw=full*0.47f, rightw=full-leftw-gap;
    ImGui::SetCursorPosX(16);
    panel("##library",{leftw,322});
    section("01","SKIN LIBRARY");
    ImGui::SetNextItemWidth(leftw-20);
    ImGui::InputTextWithHint("##search","Search weapon or finish...",search_text,sizeof(search_text));
    ImGui::SetNextItemWidth(leftw-20);
    ImGui::Combo("##filter",&category,categories,(int)std::size(categories));
    ImGui::BeginChild("##skinlist",{0,210},false);
    int visible=0;
    for(int i=0;i<(int)items.size();++i){
        auto& x=items[i];
        if(category>0&&x.category!=categories[category])continue;
        if(search_text[0]&&x.name.find(search_text)==std::string::npos)continue;
        ImGui::PushID(i);
        const bool active=selected_item==i;
        ImGui::PushStyleColor(ImGuiCol_Header,active?ImVec4{accent.x,accent.y,accent.z,.22f}:ImVec4{.12f,.13f,.17f,.55f});
        if(ImGui::Selectable("##entry",active,0,{0,42}))selected_item=i;
        ImGui::PopStyleColor();
        ImGui::SameLine(8);
        ImGui::BeginGroup();
        ImGui::Text("%s",x.name.c_str());
        ImGui::TextDisabled("%s  ·  %.4f",x.category.c_str(),x.wear);
        ImGui::EndGroup();
        ImGui::PopID(); ++visible;
    }
    if(!visible)ImGui::TextDisabled("No matching skins in this profile.");
    ImGui::EndChild();
    ImGui::SetNextItemWidth((leftw-30)*.58f);
    ImGui::Combo("##addcategory",&category,categories+1,(int)std::size(categories)-1);
    ImGui::SameLine();
    ImGui::SetNextItemWidth((leftw-30)*.40f);
    ImGui::InputTextWithHint("##newskin","Add finish...",item_name,sizeof(item_name));
    if(ImGui::Button("＋  Add to collection")&&item_name[0]){
        items.push_back({categories[std::clamp(category+1,1,(int)std::size(categories)-1)],item_name,item_wear,item_seed,item_stattrak});
        selected_item=(int)items.size()-1; item_name[0]=0; message="Skin added to local collection";
    }
    end_panel();

    ImGui::SameLine(0,gap);
    panel("##editor",{rightw,322});
    section("02","FINISH EDITOR");
    const std::string weapon=selected_item>=0&&selected_item<(int)items.size()?items[selected_item].name:std::string("Choose a skin");
    ImGui::TextColored(ImVec4{.94f,.94f,.98f,1},"%s",weapon.c_str());
    ImGui::TextDisabled("WEAPON PREVIEW  /  STATIC ART");
    ImVec2 art=ImGui::GetCursorScreenPos();
    weapon_art(art,{rightw-22,78},selected_finish);
    ImGui::Dummy({rightw-22,78});
    ImGui::Spacing();
    if(selected_item>=0&&selected_item<(int)items.size()){
        auto& x=items[selected_item];
        ImGui::SetNextItemWidth(rightw-22); ImGui::SliderFloat("Wear / Float",&x.wear,0.0f,1.0f,"%.4f");
        ImGui::SetNextItemWidth(rightw-22); ImGui::SliderInt("Pattern seed",&x.seed,0,1000);
        ImGui::Checkbox("StatTrak", &x.stattrak);
        ImGui::SameLine(0,22); ImGui::TextDisabled("%s",x.stattrak?"TRACKING ENABLED":"STANDARD FINISH");
        if(ImGui::Button("Remove selected")&&selected_item<(int)items.size()){
            items.erase(items.begin()+selected_item); selected_item=items.empty()?-1:std::min(selected_item,(int)items.size()-1); message="Skin removed";
        }
    }else ImGui::TextDisabled("Select an entry from the library to edit its finish values.");
    end_panel();

    ImGui::SetCursorPosX(16); ImGui::Spacing();
    section("03","QUICK FINISHES");
    ImGui::SameLine(0,12); ImGui::TextDisabled("Choose a visual reference");
    for(int i=0;i<10;++i){
        if(i%5)ImGui::SameLine(0,7);
        ImGui::PushID(100+i);
        const bool active=selected_finish==i;
        ImGui::PushStyleColor(ImGuiCol_Button,active?ImVec4{accent.x,accent.y,accent.z,.34f}:ImVec4{.12f,.13f,.17f,.95f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4{accent.x,accent.y,accent.z,.48f});
        if(ImGui::Button(finishes[i],{(full-28)/5,30}))selected_finish=i;
        ImGui::PopStyleColor(2); ImGui::PopID();
    }
    ImGui::SetCursorPosX(16); ImGui::Spacing();
    if(ImGui::Button("Export .vsl"))save_profile();
    ImGui::SameLine(); if(ImGui::Button("Import .vsl"))load_profile();
    ImGui::SameLine(); if(ImGui::Button("Clear profile")){items.clear();selected_item=-1;message="Profile cleared";}
    ImGui::SameLine(); ImGui::TextDisabled("%s",message.c_str());
    ImGui::SetCursorPosX(16);
    ImGui::TextDisabled("LOCAL PROFILE / Static preview only. No in-game item changes are performed.");
    ImGui::PopStyleVar(3);
}
} // namespace ui
