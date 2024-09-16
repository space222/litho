#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include "stb_image.h"
#include "vector.h"

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s64 = int64_t;
using s32 = int32_t;
using s16 = int16_t;
using s8 = int8_t;

struct point { u32 x, y; };
float sample(u8*, const point&, u32);
void calc_normal(float n[3], float x0, float y0, float z0,  float x1, float y1, float z1,  float x2, float y2, float z2);
void gen_box(std::vector<vec3>&, vec3 center, vec3 size, int);

const float eighth = 1/8.f;
u32 maxval = 0;

int main(int argc, char** args)
{
	if( argc < 4 )
	{
		printf("litho filename width height\n");
		return 1;	
	}
	
	u32 img_w,img_h,n;
	u8* data = stbi_load(args[1], (int*)&img_w, (int*)&img_h, (int*)&n, 4);
	if( !data )
	{
		printf("Unable to load %s\n", args[1]);
		return 1;
	}
	
	for(u32 y = 0; y < img_h; ++y)
	{
		for(u32 x = 0; x < img_w; ++x)
		{
			u32 R = data[(y*img_w + x)*4];
			u32 G = data[(y*img_w + x)*4+1];
			u32 B = data[(y*img_w + x)*4+2];
			R += G + B;
			R /= 3;
			if( R < 0xff && R > maxval ) maxval = R;
		}
	}
	
	const float print_w = std::atoi(args[2]);
	const float print_h = std::atoi(args[3]);
	if( print_w == 0 || print_h == 0 )
	{
		printf("Bad dimensions\n");
		return 1;
	}
	
	std::string oname(args[1]);
	if( auto pos = oname.rfind('.'); pos != std::string::npos ) oname.erase(pos);
	oname += ".stl";
	FILE* fstl = fopen(oname.c_str(), "wb");
	if( !fstl )
	{
		printf("Unable to open output file '%s'\n", oname.c_str());
		return 1;	
	}
	
	std::vector<vec3> stl_data;
	int num_tris = 0;
	
	// generate the actual litho surface	
	for(u32 img_y = 0; img_y < img_h-1; ++img_y)
	{
		for(u32 img_x = 1; img_x < img_w; ++img_x)
		{
			float z0 = sample(data, { img_x, img_y }, img_w);
			float z1 = sample(data, { img_x-1, img_y }, img_w);
			float z2 = sample(data, { img_x-1, img_y+1 }, img_w);
			
			float x0 = (float(img_x)/img_w)*print_w;
			float x1 = (float(img_x-1)/img_w)*print_w;
			float x2 = x1;
			
			float y0 = (float(img_y)/img_h)*print_h;
			float y1 = y0;
			float y2 = (float(img_y+1)/img_h)*print_h;
			
			float n[3];
			calc_normal(n, x0,z0,y0, x1,z1,y1, x2,z2,y2);	
			stl_data.push_back({n[0],n[1],n[2]});
			stl_data.push_back({x0,z0,print_h-y0});
			stl_data.push_back({x1,z1,print_h-y1});
			stl_data.push_back({x2,z2,print_h-y2});
			num_tris += 1;				
		}
		for(u32 img_x = 0; img_x < img_w-1; ++img_x)
		{
			float z0 = sample(data, { img_x, img_y+1 }, img_w);
			float z1 = sample(data, { img_x+1, img_y+1 }, img_w);
			float z2 = sample(data, { img_x+1, img_y }, img_w);
			
			float x0 = (float(img_x)/img_w)*print_w;
			float x1 = (float(img_x+1)/img_w)*print_w;
			float x2 = x1;
			
			float y0 = (float(img_y+1)/img_h)*print_h;
			float y1 = y0;
			float y2 = (float(img_y)/img_h)*print_h;

			float n[3];
			calc_normal(n, x0,z0,y0, x1,z1,y1, x2,z2,y2);			
			stl_data.push_back({n[0],n[1],n[2]});
			stl_data.push_back({x0,z0,print_h-y0});
			stl_data.push_back({x1,z1,print_h-y1});
			stl_data.push_back({x2,z2,print_h-y2});
			num_tris += 1;
		}
	}
	
	// generate back panel
	stl_data.push_back({ 0, -1, 0 });
	stl_data.push_back({ 0, 0, 0 });
	stl_data.push_back({ print_w, 0, 0 });
	stl_data.push_back({ print_w, 0, print_h });
	
	stl_data.push_back({ 0, -1, 0 });
	stl_data.push_back({ print_w, 0, print_h });
	stl_data.push_back({ 0, 0, print_h });
	stl_data.push_back({ 0, 0, 0 });
	num_tris += 2;

	const float e = eighth+0.01f;

	// generate top panel
	stl_data.push_back({ 0, 0, 0 });
	stl_data.push_back({ print_w, 0, print_h });
	stl_data.push_back({ 0, 0, print_h });
	stl_data.push_back({ 0, e, print_h });
	stl_data.push_back({ 0, 0, 0 });
	stl_data.push_back({ 0, e, print_h });
	stl_data.push_back({ print_w, e, print_h });
	stl_data.push_back({ print_w, 0, print_h });
	num_tris += 2;

	// generate bottom panel
	stl_data.push_back({ 0, 0, 0 });
	stl_data.push_back({ print_w, 0, 0 });
	stl_data.push_back({ 0, 0, 0 });
	stl_data.push_back({ 0, e, 0 });
	stl_data.push_back({ 0, 0, 0 });
	stl_data.push_back({ 0, e, 0 });
	stl_data.push_back({ print_w, e, 0 });
	stl_data.push_back({ print_w, 0, 0 });
	num_tris += 2;
	
	// generate left panel
	stl_data.push_back({ 0, 0, 0 });
	stl_data.push_back({ 0, 0, 0 });
	stl_data.push_back({ 0, e, 0 });
	stl_data.push_back({ 0, 0, print_h });
	stl_data.push_back({ 0, 0, 0 });
	stl_data.push_back({ 0, 0, print_h });
	stl_data.push_back({ 0, e, 0 });
	stl_data.push_back({ 0, e, print_h });
	num_tris += 2;
	
	// generate right panel
	stl_data.push_back({ 0, 0, 0 });
	stl_data.push_back({ print_w, 0, 0 });
	stl_data.push_back({ print_w, e, 0 });
	stl_data.push_back({ print_w, 0, print_h });
	stl_data.push_back({ 0, 0, 0 });
	stl_data.push_back({ print_w, 0, print_h });
	stl_data.push_back({ print_w, e, 0 });
	stl_data.push_back({ print_w, e, print_h });
	num_tris += 2;
	
	// turns out having interior faces "turns off" slicing resulting in holes
	// so can't use boxes "phased" slightly into the model to solidify
	// even added the ignore face parameter to try to fix that, but that didn't work either.
	// The above 4 "panels" just kinda sit vaguely flush with the litho and back panel
	// and for some reason, the slicer then sees that as a solid. Still stuck moving the model
	// down a bit to get an error about lacking a first layer, but at least have a solid now.
	// generate frame boxes
	//gen_box(stl_data, { print_w/2, eighth/2, print_h-eighth/2 }, { print_w-eighth, eighth+0.01f, eighth+0.01f }, 5); // top
	//gen_box(stl_data, { print_w/2, eighth/2, eighth/2 }, { print_w-eighth, eighth+0.01f, eighth+0.01f }, 3);         // bottom
	//num_tris += 22;
	//gen_box(stl_data, { 0, eighth/2, print_h/2 }, { eighth+0.01f, eighth+0.01f, print_h-eighth }, 2);        // left
	//gen_box(stl_data, { print_w, eighth/2, print_h/2 }, { eighth+0.01f, eighth+0.01f, print_h-eighth }, 4);  // right
	//num_tris += 22;
	
	// output the STL file
	// 80 byte header goes ignored by most software	
	for(u32 i = 0; i < 80; ++i) fputc(0, fstl);
	// number of triangles
	fwrite(&num_tris, 1, 4, fstl);
	// if there wasn't an unused 16bit attribute count per triangle, this would be easier
	for(int i = 0; i < num_tris; ++i)
	{
		fwrite(stl_data.data()+i*4, 1, 12*4, fstl);
		short a = 0;
		fwrite(&a, 1, 2, fstl);
	}
	fclose(fstl);
	
	return 0;
}

float sample(u8* data, const point& p, u32 img_w)
{
	const float maxdepth = eighth - 0.02f;
	
	u32 R = data[(p.y*img_w + p.x)*4];
	u32 G = data[(p.y*img_w + p.x)*4 + 1];
	u32 B = data[(p.y*img_w + p.x)*4 + 2];
	float avg = R + G + B;
	avg /= 3;
	return eighth - (maxdepth * (avg/float(maxval))); //255.f));
}

void calc_normal(float n[3], float x0, float y0, float z0,  float x1, float y1, float z1,  float x2, float y2, float z2)
{       // turns out 3d printing software doesn't need/care about the normal (right?)
	float Ux = x0 - x1;
	float Uy = y0 - y1;
	float Uz = z0 - z1;
	float Vx = x2 - x1;
	float Vy = y2 - y1;
	float Vz = z2 - z1;
	n[0] = Uy*Vz - Uz*Vy;
	n[1] = Uz*Vx - Ux*Vz;
	n[2] = Ux*Vy - Uy*Vx;
}

// adapted from https://github.com/prideout/par/blob/master/par_shapes.h
static vec3 verts[8] = {
	{ -0.5f, -0.5f, -0.5f },
	{ -0.5f, 0.5f, -0.5f },
	{ 0.5f, 0.5f, -0.5f },
	{ 0.5f, -0.5f, -0.5f },
	{ -0.5f, -0.5f, 0.5f },
	{ -0.5f, 0.5f, 0.5f },
	{ 0.5f, 0.5f, 0.5f },
	{ 0.5f, -0.5f, 0.5f },
};
static u32 quads[6 * 4] = {
	7,6,5,4, // front
	0,1,2,3, // back
	6,7,3,2, // right
	5,6,2,1, // top
	4,5,1,0, // left
	7,4,0,3, // bottom
};

void gen_box(std::vector<vec3>& sd, vec3 c, vec3 sz, int ignore)
{
	auto quad = &quads[0];
	for(int p = 0; p < 6; ++p, quad += 4) 
	{
		if( p == ignore ) continue;
        	sd.push_back({0,0,0});
        	sd.push_back(c + sz*verts[quad[0]]);
        	sd.push_back(c + sz*verts[quad[1]]);
        	sd.push_back(c + sz*verts[quad[2]]);    
		sd.push_back({0,0,0});
        	sd.push_back(c + sz*verts[quad[2]]);
        	sd.push_back(c + sz*verts[quad[3]]);
        	sd.push_back(c + sz*verts[quad[0]]);    
	}
}

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"














