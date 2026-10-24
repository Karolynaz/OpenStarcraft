#include "menu_animation.h"
#include <cassert>
#include <set>
namespace fake {std::atomic<unsigned> queued{0},allocations{0},pauses{0};std::vector<Draw> draws;}
int main(int argc,char** argv){assert(argc==2);struct Entry{char name[40];unsigned w,h,count,offset;};FILE* f=fopen(argv[1],"rb");assert(f);char magic[8];unsigned n;assert(fread(magic,1,8,f)==8 && fread(&n,4,1,f)==1);std::vector<Entry> entries(n);assert(fread(entries.data(),sizeof(Entry),n,f)==n);unsigned frames=0,checks=0;
 {port3ds::MenuAnimation animation;assert(animation.load(argv[1]));assert(fake::allocations==14);
 for(auto& e:entries){std::set<unsigned> hashes;unsigned w,h;assert(animation.size(e.name,w,h) && w==e.w && h==e.h);
  for(unsigned frame=0;frame<e.count;++frame){std::vector<unsigned short> src(w*h);assert(!fseek(f,e.offset+frame*w*h*2,SEEK_SET));assert(fread(src.data(),2,src.size(),f)==src.size());unsigned hash=2166136261u;for(auto v:src)hash=(hash^v)*16777619u;hashes.insert(hash);
   fake::draws.clear();assert(animation.draw(e.name,0,frame,12,22));assert(fake::draws.size()==1);auto& d=fake::draws[0];auto* pixels=static_cast<unsigned short*>(d.tex->data);
   for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x){const unsigned m=(x&1)|((y&1)<<1)|((x&2)<<1)|((y&2)<<2)|((x&4)<<2)|((y&4)<<3);assert(pixels[((y/8)*16+x/8)*64+m]==src[y*w+x]);++checks;}++frames;
  }assert(hashes.size()>1);
 }
 animation.clear();assert(fake::allocations==0);assert(!animation.load("/tmp/nonexistent-sc3ds-animation-file"));}
 fclose(f);assert(fake::allocations==0);printf("%u complete animation cycles / %u frames / %u GPU texels / bounded14 surfaces released PASS\n",n,frames,checks);
}
