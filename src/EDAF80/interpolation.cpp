#include "interpolation.hpp"

glm::vec3
interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{
	// Linear interpolation formula
	return (1.0f - x) * p0 + x * p1;
}

glm::vec3
interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
                              glm::vec3 const& p2, glm::vec3 const& p3,
                              float const t, float const x)
{
	// Create the Catmull-Rom matrix
	glm::mat4 catmull_rom_matrix = glm::mat4(
											 0.0f, 1.0f, 0.0f, 0.0f,
											 -t, 0.0f, t, 0.0f,
											 2.0f * t, t - 3.0f, 3.0f - 2.0f * t, -t,
											 -t, 2.0f - t, t - 2.0f, t
											 );
	
	// Create the x powers vector: [1, x, x^2, x^3]
	glm::vec4 x_powers = glm::vec4(1.0f, x, x * x, x * x * x);
	
	// Create the vector of control points
	glm::mat4x3 points = glm::mat4x3(p0, p1, p2, p3);
	
	// Perform the matrix multiplication to get the result
	return x_powers * catmull_rom_matrix * points;
}
