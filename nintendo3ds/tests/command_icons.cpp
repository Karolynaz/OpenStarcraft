#include "graphics.h"
#include "bwenums.h"
#include "data_loading.h"
#include <atomic>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>
namespace fake {std::atomic<unsigned> queued{0},allocations{0},pauses{0};std::vector<Draw> draws;}
static unsigned u32(std::ifstream& f){unsigned char b[4];f.read((char*)b,4);assert(f);return b[0]|b[1]<<8|b[2]<<16|unsigned(b[3])<<24;}
static std::vector<uint16_t> pixels(std::ifstream& f,unsigned count){std::vector<uint16_t> result(count);for(auto& p:result){const unsigned a=f.get(),b=f.get();assert(f);p=a|b<<8;}return result;}
static std::vector<uint16_t> capture(const fake::Draw& d){
 const unsigned x=unsigned(d.sub.left*512+0.5f),y=unsigned((1-d.sub.top)*512+0.5f);const auto* tex=(const uint16_t*)d.tex->data;std::vector<uint16_t> out;
 for(unsigned row=0;row<d.sub.height;++row)for(unsigned col=0;col<d.sub.width;++col){const unsigned px=x+col,py=y+row,m=(px&1)|((py&1)<<1)|((px&2)<<1)|((py&2)<<2)|((px&4)<<2)|((py&4)<<3);out.push_back(tex[((py/8)*64+px/8)*64+m]);}return out;
}
int main(int argc,char** argv){
 assert(argc==5);using namespace port3ds;using namespace bwgame;
 std::ifstream reference(argv[4],std::ios::binary);char magic[8];reference.read(magic,8);assert(std::string(magic,7)=="OBWUI02");assert(u32(reference)==3);const auto frames=u32(reference);assert(frames==390);
 const Scenario labs[]={Scenario::zerg_lab,Scenario::terran_lab,Scenario::protoss_lab},battles[]={Scenario::zerg_battle,Scenario::terran_battle,Scenario::protoss_battle};
 auto files=data_loading::data_files_directory(argv[1]);a_vector<uint8_t> bytes;files(bytes,"arr/techdata.dat");const auto techs=data_loading::load_techdata_dat(bytes);files(bytes,"arr/upgrades.dat");const auto upgrades=data_loading::load_upgrades_dat(bytes);
 std::string base=argv[4];base=base.substr(0,base.find_last_of('/')+1);std::ifstream hud_reference(base+"resources.bin",std::ios::binary);assert(hud_reference);
 unsigned compared=0,options_checked=0,viewports=0,ui_draws=0;
 for(unsigned race=0;race<3;++race){
  Engine engine;assert(engine.load(argv[1],argv[2],labs[race]));for(unsigned i=0;i<24;++i)assert(engine.tick());
  const auto resources=engine.snapshot();
  std::vector<uint16_t> actual(48*40);
  for(unsigned frame=0;frame<frames;++frame){
   const unsigned x=u32(reference),y=u32(reference),w=u32(reference),h=u32(reference);const auto info=engine.sprite_info(engine.command_icon(frame).image,frame);assert(info.valid && info.width==w && info.height==h);
   for(unsigned disabled=0;disabled<2;++disabled){
    const auto expected=pixels(reference,w*h);const auto icon=engine.command_icon(frame,!disabled);assert(icon.x==int(x) && icon.y==int(y) && icon.frame==frame);
    assert(engine.sprite_pixels(icon.image,icon.frame,icon.owner,actual.data(),actual.size()));for(unsigned p=0;p<w*h;++p)assert(actual[p]==expected[p]);compared+=w*h;
   }
  }
  const unsigned resource_frames[3][3]={{0,1,4},{0,2,5},{0,3,6}};
  for(unsigned frame=0;frame<7;++frame){const auto expected=pixels(hud_reference,196);auto view=engine.resource_icon(0);assert(view.image);view.frame=frame;
   const auto info=engine.sprite_info(view.image,frame);assert(info.valid && info.width==14 && info.height==14);
   assert(engine.sprite_pixels(view.image,frame,view.owner,actual.data(),actual.size()));assert(std::equal(expected.begin(),expected.end(),actual.begin()));compared+=196;
  }
  for(unsigned kind=0;kind<3;++kind)assert(engine.resource_icon(kind).frame==resource_frames[race][kind]);
  assert(!engine.resource_icon(3).image);
  const auto expected_button=pixels(reference,48*40);const auto button=engine.command_button();assert(engine.sprite_pixels(button.image,0,11,actual.data(),actual.size()) && actual==expected_button);compared+=48*40;
  assert(engine.command_icon(frames).image==0 && !engine.sprite_pixels(button.image,0,11,actual.data(),10));
  assert(engine.snapshot().minerals==resources.minerals && engine.snapshot().gas==resources.gas && engine.snapshot().frame==resources.frame);
  UnitView units[512];const auto n=engine.visible_units(units,512);std::set<unsigned> visited;
  for(unsigned i=0;i<n;++i)if(units[i].owner==0 && visited.insert(units[i].type).second && engine.select(units[i].x,units[i].y))for(auto page:{OptionPage::primary,OptionPage::secondary,OptionPage::basic,OptionPage::advanced}){
   MenuOption rows[8];const auto count=engine.options(page,rows,8);for(unsigned j=0;j<count;++j){
    const auto kind=OptionKind(rows[j].code>>16);const unsigned id=rows[j].code&65535;
    if(kind==OptionKind::build || kind==OptionKind::train || kind==OptionKind::addon || kind==OptionKind::morph)assert(rows[j].icon==id);
    if(kind==OptionKind::research)assert(rows[j].icon==unsigned(techs.vec[id].icon));
    if(kind==OptionKind::upgrade)assert(rows[j].icon==unsigned(upgrades.vec[id].icon));
    if(rows[j].icon!=~0u){const auto icon=engine.command_icon(rows[j].icon,rows[j].enabled);assert(icon.image && engine.sprite_info(icon.image,icon.frame).valid);++options_checked;}
   }
  }
  assert(engine.load(argv[1],argv[2],battles[race]));Graphics graphics;assert(graphics.load(argv[3],engine.terrain_signature()));
  for(unsigned view=0;view<25;++view){
   for(unsigned i=0;i<24;++i)assert(engine.tick());const int cx=(view%5)*400,cy=(view/5)*360;
   SpriteView images[2048];const auto count=engine.visible_images(images,2048,cx,cy);fake::draws.clear();assert(graphics.sprites(images,count,cx,cy,engine)==0);
   const auto world=fake::draws;std::vector<std::vector<uint16_t>> before;for(const auto& d:world)before.push_back(capture(d));
   fake::binding().check=true;
   for(unsigned i=0;i<8;++i){assert(graphics.ui_image(engine.command_button(),136+i%2*92,144+i/2*24,0.55f,engine));assert(graphics.ui_image(engine.command_icon((view*8+i)%frames,i%2==0),140+i%2*92,147+i/2*24,0.55f,engine));ui_draws+=2;}
   for(unsigned kind=0;kind<3;++kind){const auto icon=engine.resource_icon(kind);assert(graphics.ui_image(icon,250+kind*48,2,1,engine));const auto& d=fake::draws.back();assert(d.x==250+kind*48 && d.y==2 && d.sub.width==14 && d.sub.height==14 && d.sx==1);++ui_draws;}
   assert(fake::binding().stale==0);fake::binding().check=false;
   for(unsigned i=0;i<world.size();++i)assert(capture(world[i])==before[i]);++viewports;
  }
  // Fill enough distinct current-frame icons to cross atlas pages without
  // intervening text. The second upload to the same later page must preserve
  // Citro2D's cached binding, not force the first atlas page underneath it.
  graphics.clear();assert(graphics.load(argv[3],engine.terrain_signature()));fake::draws.clear();fake::binding().check=true;
  std::set<C3D_Tex*> icon_pages;for(unsigned i=0;i<frames;++i){assert(graphics.ui_image(engine.command_icon(i),0,0,1,engine));icon_pages.insert(fake::draws.back().tex);}
  assert(icon_pages.size()>=2 && fake::binding().stale==0);fake::binding().check=false;
 }
 std::printf("UI coverage: %u mapped menu options, allocations %u, EOF %d\n",options_checked,fake::allocations.load(),reference.peek()==std::char_traits<char>::eof());
 assert(hud_reference.peek()==std::char_traits<char>::eof() && reference.peek()==std::char_traits<char>::eof() && fake::allocations==0 && options_checked>150);
 std::printf("Original UI GRP/PCX/remap/coverage: %u RGBA pixels, %u native catalogue icons, %u battle viewports / %u UI draws preserve queued world textures; all allocations freed PASS\n",compared,options_checked,viewports,ui_draws);
}
