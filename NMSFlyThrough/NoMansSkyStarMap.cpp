#include "NoMansSkyStarMap.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <regex>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Helpers/ErrorHandling.h>
#include <Helpers/Utils.h>
#include <Helpers/Shader.h>
#include <Renderables/TextBillboardDynamic.h>
#include <Renderables/TextBillboardFixed.h>

NoMansSky::NoMansSky()
	: mGridShader(new Shader("Lines.v.glsl", "Lines.f.glsl", ""))
	, mStarShader(new Shader("points.v.glsl", "points.f.glsl", "points.g.glsl"))
{
}

#define DEBUG_GRID
#define DEBUG_STARS

void NoMansSky::setUp(const std::string& fileName, const AABB& world)
{
	glm::u32vec3 gridSizes({ 0xAA, 0xAA, 0xAA });

	glGenVertexArrays(2, &mVao[0]);
	

	AABB NMSSize(glm::vec4(-0x7FF, -0x7F, -0x7FF, 1),
				 glm::vec4(0x7FF, 0x7F, 0x7FF, 1));
	float scaleNMStoWorld = world.size().x / NMSSize.size().x;
#ifdef DEBUG_GRID
	createGrid(NMSSize, gridSizes, scaleNMStoWorld);
	glBindVertexArray(mVao[0]);

	GLuint vboGrid;
	glGenBuffers(1, &vboGrid);
	glBindBuffer(GL_ARRAY_BUFFER, vboGrid);
	mGridShader->use();
	GLuint location = mGridShader->getAttributeLocation("vPos");
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mGrid.size(), 
					mGrid.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glLineWidth(1.0f);
	glEnableVertexAttribArray(location);
	glBindVertexArray(0);
#endif

#ifdef DEBUG_STARS
	loadStars(fileName, NMSSize, scaleNMStoWorld);
	glBindVertexArray(mVao[1]);
	GLuint vboStar;
	glGenBuffers(1, &vboStar);
	glPointSize(30.0f);
	glBindBuffer(GL_ARRAY_BUFFER, vboStar);
	location = mStarShader->getAttributeLocation("pointpos");
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * mStarPositions.size(), 
			mStarPositions.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
#endif
}

void NoMansSky::createGrid(const AABB& nmsSpace,
						   const glm::u32vec3& gridSizes,
						   float scaleToWorld)
{
	// https://nomanssky.gamepedia.com/Portal_address

	for (glm::vec4::value_type x = nmsSpace.minPoint().x;
			x <= nmsSpace.maxPoint().x; x += gridSizes.x)
	{
		const float xf = x * scaleToWorld;
		for (glm::vec4::value_type y = nmsSpace.minPoint().y;
				y <= nmsSpace.maxPoint().y; y += gridSizes.y)
		{
			const float yf = y * scaleToWorld;
			for (glm::vec4::value_type z = nmsSpace.minPoint().z;
					z <= nmsSpace.maxPoint().z; z += gridSizes.z)
			{
				const float zf = z * scaleToWorld;

				mGrid.push_back({ xf, nmsSpace.minPoint().y * scaleToWorld, zf });
				mGrid.push_back({ xf, nmsSpace.maxPoint().y * scaleToWorld, zf });

				// horozontal lines
				mGrid.push_back({ nmsSpace.minPoint().x * scaleToWorld, yf, zf });
				mGrid.push_back({ nmsSpace.maxPoint().x * scaleToWorld, yf, zf });

				mGrid.push_back({ xf, yf, nmsSpace.minPoint().z * scaleToWorld });
				mGrid.push_back({ xf, yf, nmsSpace.maxPoint().z * scaleToWorld });
			}
		}
	}
}

float normalise(float value, float max)
{
	return float(2.0 * value / max - 1.0);
}

void NoMansSky::loadStars(const std::string& fileName,
						  const AABB& nmsSpace,
						  float scaleToWorld)
{
	int fontHeight = 12;
	glm::vec2 nice = { 0.00625f, 2 * 0.00625f };

	std::string line;
	std::ifstream myfile(fileName);
	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			if (line.size() == 0) // Test for empty line (CR?)
				break;
			if (line[0] == '#')
				continue;
			glm::vec4 point(0);
			std::vector<std::string> elms = split(line, ',');
			TextBillboard* text = new TextBillboardFixed("arial.ttf", fontHeight);
			text->createText(elms[0], nice.x, nice.y);
			text->color({ 0.0, 0.0, 0.0, 1.0f });
			SimpleVertexRender svr;
			svr.setUp(text);
			mStarLabels.push_back(svr);

			if (elms.size() == 2)
			{
				std::vector<std::string> coords = split(elms[1], ':');
				for (auto& s : coords)
					trim(s);
				if (coords.size() == 5)
				{
					// Signal Booster: AAAA:XXXX:YYYY:ZZZZ:SSSS
					// AAAA - something to do with the location on a planet's surface
					// SSSS - is the index of the system within the galactic region. Each region is, roughly, a 400 LY cube.
					//      - Same as Portal address System ID
					coords.erase(coords.begin());
				}

				if (coords.size() == 4)
				{
					// https://nomanssky.gamepedia.com/Portal_address#Galactic_Coordinates_System
					// Values begin at the bottom northwest corner of the chart and end at the 
					// top southeast corner, at Delta Majoris, where it ends 
					// at 0FFE:00FE : 0FFE : 0000
					// format is label, galactic coords: screenX:screenY:z:w
					// This mapping to the Game world asuming 0,0,0 is at the center.
					__int64 x = std::stoull(coords[0], 0, 16) - 0x7FF;
					__int64 y = std::stoull(coords[1], 0, 16) - 0x7F;
					__int64 z = std::stoull(coords[2], 0, 16) - 0x7FF;
					point.x = static_cast<glm::vec4::value_type>(x);
					point.y = static_cast<glm::vec4::value_type>(y);
					point.z = static_cast<glm::vec4::value_type>(z);
				}
				else if (coords.size() == 1)
				{
					// https://nomanssky.gamepedia.com/Portal_address#Portal_Coordinates_System
					std::string portalNum = coords[0];
					if (portalNum.size() == 12)
					{
						// Portal Coordinates can be confirmed at 
						// https://pahefu.github.io/pilgrimstarpath/
												
						// format is label, PortalAddress. Comes from the 12 Glyph 
						// characters that a Portal Terminal uses. Format is:
						// Planet Index(1)/Solar System Index(3)/Voxel Position(8)
						// Voxel Position x=0, y=0, z=0 is at the center.
						//				Y Values. Y axis is height. Ranges from [01, FF)
						// Increases from 0 to Up Pole at 7F
						// Starts again at Down Pole at 81 increasing to FF (which is back at the center)
						// Entering Y=80 returns Y=81.
						//				X Values. [001, FFF]
						// X Values have East and West Poles. They increase easterly beginning 
						// with 0 at the center. 7FF is  the East (right) Pole
						// Resumes with West Pole = X=801 and increasing towards the center.
						// Entering X=800 returns X=801
						//				Z Values. [001, FFF] (details same as X)
						// Z values have South and North Poles. They increase southerly until
						// z=7FF then numbering resumes from the northern pole with Z=801 and 
						// increasing as it nears centre

						// PSSSYYZZZXXX
							
						// P -  Planet Index in a system/ Max value == 6
						std::string planetIndex = portalNum.substr(0, 1);
						// SSS - Solar System Index. The address for a specific 
						// star system within the given region. SO far the max 
						// number of systems in a region iss 555
						std::string solarSystemIndex = portalNum.substr(1, 3);

						// x,y,z voxel positions
						std::string sY = portalNum.substr(4, 2);
						std::string sZ = portalNum.substr(6, 3);
						std::string sX = portalNum.substr(9, 3);

						// This mapping to the Game world asuming 0,0,0 is at the center.
						__int64 x = std::stoull(sX, 0, 16);
						if (x > 0x7FF)
							x -= 0xFFF;

						__int64 y = std::stoull(sY, 0, 16);
						if (y > 0x7F)
							y -= 0xFF;

						__int64 z = std::stoull(sZ, 0, 16);
						if (z > 0x7FF)
							z -= 0xFFF;
						point.x = static_cast<glm::vec4::value_type>(x);
						point.y = static_cast<glm::vec4::value_type>(y);
						point.z = static_cast<glm::vec4::value_type>(z);

					}
				}
			}
			else if (elms.size() == 4)
			{
				// format is label, screenX, screenY, z
				point.x = static_cast<glm::vec4::value_type>(std::stoull(elms[1]));
				point.y = 0;
				point.z = static_cast<glm::vec4::value_type>(std::stoull(elms[2]));
			}
			point.x *= scaleToWorld;
			point.y *= scaleToWorld;
			point.z *= scaleToWorld;
			point.w = 1.0;
			mStarPositions.push_back({ point.x, point.y, point.z, point.w });
		}

		myfile.close();
	}
	else
	{
		throw NMSException(std::stringstream() << "File [" << fileName << "] not loaded.");
	}
}

void NoMansSky::render(const glm::mat4& proj, const glm::mat4& view, const glm::mat4& model)
{
	glm::mat4 pvm = proj * view * model;
#ifdef DEBUG_GRID
	mGridShader->use();
	checkGLError();
	mGridShader->setMatrix4("pvm", pvm);
	checkGLError();
	glBindVertexArray(mVao[0]);
	checkGLError();
	mGridShader->setVector4f("uColour", { 0, 1.0, 0.5, 1 });
	checkGLError();
	glDrawArrays(GL_LINES, 0, GLsizei(mGrid.size()));
	checkGLError();
	glBindVertexArray(0);
#endif

#ifdef DEBUG_STARS
	mStarShader->use();
	checkGLError();
	mStarShader->setMatrix4("pvm", pvm);
	checkGLError();
	glBindVertexArray(mVao[1]);
	checkGLError();
	mStarShader->setVector4f("pointColour", { 0.6, 0.0, 0.3, 1 });
	glDrawArrays(GL_POINTS, 0, GLsizei(mStarPositions.size()));
	checkGLError();
	glBindVertexArray(0);
	for (int i = 0; i < mStarLabels.size(); i++)
	{
		const SimpleVertexRender& svr = mStarLabels[i];
		const glm::vec4& v4 = mStarPositions[i];
		glm::mat4 temp = glm::translate(model, glm::vec3(v4.x, v4.y, v4.z));
		svr.render(proj, view, temp);
	}
#endif
	checkGLError();
}
