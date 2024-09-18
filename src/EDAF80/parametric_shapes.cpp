#include "parametric_shapes.hpp"
#include "core/Log.h"

#include <glm/glm.hpp>

#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

bonobo::mesh_data
parametric_shapes::createQuad(float const width, float const height,
                              unsigned int const horizontal_split_count,
                              unsigned int const vertical_split_count)
{
	auto const vertices = std::array<glm::vec3, 4>{
		glm::vec3(0.0f,  0.0f,   0.0f),
		glm::vec3(width, 0.0f,   0.0f),
		glm::vec3(width, height, 0.0f),
		glm::vec3(0.0f,  height, 0.0f)
	};

	auto const index_sets = std::array<glm::uvec3, 2>{
		glm::uvec3(0u, 1u, 2u),
		glm::uvec3(0u, 2u, 3u)
	};

	bonobo::mesh_data data;

	if (horizontal_split_count > 0u || vertical_split_count > 0u)
	{
		LogError("parametric_shapes::createQuad() does not support tesselation.");
		return data;
	}

	//
	// NOTE:
	//
	// Only the values preceeded by a `\todo` tag should be changed, the
	// other ones are correct!
	//

	// Create a Vertex Array Object: it will remember where we stored the
	// data on the GPU, and  which part corresponds to the vertices, which
	// one for the normals, etc.
	//
	// The following function will create new Vertex Arrays, and pass their
	// name in the given array (second argument). Since we only need one,
	// pass a pointer to `data.vao`.
	
	glGenVertexArrays(1, &data.vao);

//	glGenVertexArrays(1, /*! \todo fill me */nullptr);

	// To be able to store information, the Vertex Array has to be bound
	// first.
	glBindVertexArray(data.vao);

	// To store the data, we need to allocate buffers on the GPU. Let's
	// allocate a first one for the vertices.
	//
	// The following function's syntax is similar to `glGenVertexArray()`:
	// it will create multiple OpenGL objects, in this case buffers, and
	// return their names in an array. Have the buffer's name stored into
	// `data.bo`.
	glGenBuffers(1, &data.bo);

	// Similar to the Vertex Array, we need to bind it first before storing
	// anything in it. The data stored in it can be interpreted in
	// different ways. Here, we will say that it is just a simple 1D-array
	// and therefore bind the buffer to the corresponding target.
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
	             /* where is the data stored on the CPU? */vertices.data(),
	             /* inform OpenGL that the data is modified once, but used often */GL_STATIC_DRAW);

	// Vertices have been just stored into a buffer, but we still need to
	// tell Vertex Array where to find them, and how to interpret the data
	// within that buffer.
	//
	// You will see shaders in more detail in lab 3, but for now they are
	// just pieces of code running on the GPU and responsible for moving
	// all the vertices to clip space, and assigning a colour to each pixel
	// covered by geometry.
	// Those shaders have inputs, some of them are the data we just stored
	// in a buffer object. We need to tell the Vertex Array which inputs
	// are enabled, and this is done by the following line of code, which
	// enables the input for vertices:
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));

	// Once an input is enabled, we need to explain where the data comes
	// from, and how it interpret it. When calling the following function,
	// the Vertex Array will automatically use the current buffer bound to
	// GL_ARRAY_BUFFER as its source for the data. How to interpret it is
	// specified below:
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices),
	                      /* how many components do our vertices have? (x, y, z) */ 3,
	                      /* what is the type of each component? */GL_FLOAT,
	                      /* should it automatically normalise the values stored */GL_FALSE,
	                      /* once all components of a vertex have been read, how far away (in bytes) is the next vertex? */0,
	                      /* how far away (in bytes) from the start of the buffer is the first vertex? */reinterpret_cast<GLvoid const*>(0x0));

	// Now, let's allocate a second one for the indices.
	//
	// Have the buffer's name stored into `data.ibo`.
	glGenBuffers(1, &data.ibo);

	// We still want a 1D-array, but this time it should be a 1D-array of
	// elements, aka. indices!
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_sets),
	             /* where is the data stored on the CPU? */index_sets.data(),
	             /* inform OpenGL that the data is modified once, but used often */GL_STATIC_DRAW);

	data.indices_nb =  index_sets.size() * 3; // number of triangles and 3 vertices in each triangle

	// All the data has been recorded, we can unbind them.
	glBindVertexArray(0u);
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}

bonobo::mesh_data
parametric_shapes::createSphere(float const radius, unsigned int const horizontal_split_count, unsigned int const vertical_split_count)
{
	bonobo::mesh_data data;
	
	// Number of vertices
	// we calculate one more vertex than the number of splits to account for the edges
	const unsigned int vertex_count = (horizontal_split_count + 1) * (vertical_split_count + 1);
	
	// Arrays to store vertex attributes
	std::vector<glm::vec3> vertices(vertex_count);
	std::vector<glm::vec3> normals(vertex_count);
	std::vector<glm::vec3> tangents(vertex_count);
	std::vector<glm::vec3> binormals(vertex_count);
	std::vector<glm::vec2> texcoords(vertex_count); // Texture coordinates
	
	// Step sizes for theta and phi
	 //We divide the sphere into a grid, and d_theta and d_phi represent how much
	// we move with each step along the horizontal and vertical directions
	float const d_theta = 2.0f * glm::pi<float>() / static_cast<float>(horizontal_split_count);
	float const d_phi = glm::pi<float>() / static_cast<float>(vertical_split_count);
	
	// Generate vertices
	unsigned int index = 0;
	for (unsigned int i = 0; i <= horizontal_split_count; ++i) {
		float theta = i * d_theta;
		float cos_theta = cos(theta);
		float sin_theta = sin(theta);
		
		for (unsigned int j = 0; j <= vertical_split_count; ++j) {
			float phi = j * d_phi;
			float cos_phi = cos(phi);
			float sin_phi = sin(phi);
			
			// Compute vertex position (parametric sphere equation)
			glm::vec3 position = glm::vec3( radius * sin_phi * cos_theta,radius * cos_phi,radius * sin_phi * sin_theta);
			vertices[index] = position;
			
			// Normal is just the normalized position for a sphere
			glm::vec3 normal = glm::normalize(position);
			normals[index] = normal;
			
			glm::vec3 tangent = glm::vec3(-radius * sin_phi * sin_theta, 0.0f, radius * sin_phi * cos_theta);
			tangents[index] = glm::normalize(tangent);
			
			glm::vec3 binormal = glm::cross(tangent, normal);
			binormals[index] = glm::normalize(binormal);
			
			// Texture coordinates
			texcoords[index] = glm::vec2(static_cast<float>(i) / horizontal_split_count, static_cast<float>(j) / vertical_split_count);
			
			++index;
		}
	}
	
	// Generate indices
	std::vector<glm::uvec3> indices(horizontal_split_count * vertical_split_count * 2); // Two triangles per quad
	index = 0;
	for (unsigned int i = 0; i < horizontal_split_count; ++i) {
		for (unsigned int j = 0; j < vertical_split_count; ++j) {
			unsigned int first = i * (vertical_split_count + 1) + j;
			unsigned int second = first + vertical_split_count + 1;
			
			// First triangle
			indices[index] = glm::uvec3(first, second, first + 1);
			++index;
			
			// Second triangle
			indices[index] = glm::uvec3(second, second + 1, first + 1);
			++index;
		}
	}
	
	// Create and bind the VAO
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);
	
	// Vertex buffer size
	const auto vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
	const auto normals_size = static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
	const auto tangents_size = static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
	const auto binormals_size = static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
	const auto texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec2));
	const auto buffer_size = vertices_size + normals_size + tangents_size + binormals_size + texcoords_size;
	
	// Generate and bind the buffer object for vertices
	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_STATIC_DRAW);
	
	// Upload vertex data
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_size, vertices.data());
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));
	
	// Upload normal data
	glBufferSubData(GL_ARRAY_BUFFER, vertices_size, normals_size, normals.data());
	// copies the vertex positions into GPU memory
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	// enables the vertex attribute for positions
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_size));
	//tells OpenGL that the vertex data consists of 3D floating-point values
	
	// Upload tangent data
	glBufferSubData(GL_ARRAY_BUFFER, vertices_size + normals_size, tangents_size, tangents.data());
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_size + normals_size));
	
	// Upload binormal data
	glBufferSubData(GL_ARRAY_BUFFER, vertices_size + normals_size + tangents_size, binormals_size, binormals.data());
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_size + normals_size + tangents_size));
	
	// Upload texture coordinates
	glBufferSubData(GL_ARRAY_BUFFER, vertices_size + normals_size + tangents_size + binormals_size, texcoords_size, texcoords.data());
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(vertices_size + normals_size + tangents_size + binormals_size));
	
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	
	// Generate and bind the element buffer for indices
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(glm::uvec3), indices.data(), GL_STATIC_DRAW);
	
	data.indices_nb = static_cast<GLsizei>(indices.size() * 3u);
	
	glBindVertexArray(0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);
	
	return data;
}



bonobo::mesh_data
parametric_shapes::createTorus(float const major_radius,
                               float const minor_radius,
                               unsigned int const major_split_count,
                               unsigned int const minor_split_count)
{
	//! \todo (Optional) Implement this function
	return bonobo::mesh_data();
}

bonobo::mesh_data
parametric_shapes::createCircleRing(float const radius,
                                    float const spread_length,
                                    unsigned int const circle_split_count,
                                    unsigned int const spread_split_count)
{
	auto const circle_slice_edges_count = circle_split_count + 1u;
	auto const spread_slice_edges_count = spread_split_count + 1u;
	auto const circle_slice_vertices_count = circle_slice_edges_count + 1u;
	auto const spread_slice_vertices_count = spread_slice_edges_count + 1u;
	auto const vertices_nb = circle_slice_vertices_count * spread_slice_vertices_count;

	auto vertices  = std::vector<glm::vec3>(vertices_nb);
	auto normals   = std::vector<glm::vec3>(vertices_nb);
	auto texcoords = std::vector<glm::vec3>(vertices_nb);
	auto tangents  = std::vector<glm::vec3>(vertices_nb);
	auto binormals = std::vector<glm::vec3>(vertices_nb);

	float const spread_start = radius - 0.5f * spread_length;
	float const d_theta = glm::two_pi<float>() / (static_cast<float>(circle_slice_edges_count));
	float const d_spread = spread_length / (static_cast<float>(spread_slice_edges_count));

	// generate vertices iteratively
	size_t index = 0u;
	float theta = 0.0f;
	for (unsigned int i = 0u; i < circle_slice_vertices_count; ++i) {
		float const cos_theta = std::cos(theta);
		float const sin_theta = std::sin(theta);

		float distance_to_centre = spread_start;
		for (unsigned int j = 0u; j < spread_slice_vertices_count; ++j) {
			// vertex
			vertices[index] = glm::vec3(distance_to_centre * cos_theta,
			                            distance_to_centre * sin_theta,
			                            0.0f);

			// texture coordinates
			texcoords[index] = glm::vec3(static_cast<float>(j) / (static_cast<float>(spread_slice_vertices_count)),
			                             static_cast<float>(i) / (static_cast<float>(circle_slice_vertices_count)),
			                             0.0f);

			// tangent
			auto const t = glm::vec3(cos_theta, sin_theta, 0.0f);
			tangents[index] = t;

			// binormal
			auto const b = glm::vec3(-sin_theta, cos_theta, 0.0f);
			binormals[index] = b;

			// normal
			auto const n = glm::cross(t, b);
			normals[index] = n;

			distance_to_centre += d_spread;
			++index;
		}

		theta += d_theta;
	}

	// create index array
	auto index_sets = std::vector<glm::uvec3>(2u * circle_slice_edges_count * spread_slice_edges_count);

	// generate indices iteratively
	index = 0u;
	for (unsigned int i = 0u; i < circle_slice_edges_count; ++i)
	{
		for (unsigned int j = 0u; j < spread_slice_edges_count; ++j)
		{
			index_sets[index] = glm::uvec3(spread_slice_vertices_count * (i + 0u) + (j + 0u),
			                               spread_slice_vertices_count * (i + 0u) + (j + 1u),
			                               spread_slice_vertices_count * (i + 1u) + (j + 1u));
			++index;

			index_sets[index] = glm::uvec3(spread_slice_vertices_count * (i + 0u) + (j + 0u),
			                               spread_slice_vertices_count * (i + 1u) + (j + 1u),
			                               spread_slice_vertices_count * (i + 1u) + (j + 0u));
			++index;
		}
	}

	bonobo::mesh_data data;
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);

	auto const vertices_offset = 0u;
	auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
	auto const normals_offset = vertices_size;
	auto const normals_size = static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
	auto const texcoords_offset = normals_offset + normals_size;
	auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec3));
	auto const tangents_offset = texcoords_offset + texcoords_size;
	auto const tangents_size = static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
	auto const binormals_offset = tangents_offset + tangents_size;
	auto const binormals_size = static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
	auto const bo_size = static_cast<GLsizeiptr>(vertices_size
	                                            +normals_size
	                                            +texcoords_size
	                                            +tangents_size
	                                            +binormals_size
	                                            );
	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size, static_cast<GLvoid const*>(vertices.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));

	glBufferSubData(GL_ARRAY_BUFFER, normals_offset, normals_size, static_cast<GLvoid const*>(normals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(normals_offset));

	glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size, static_cast<GLvoid const*>(texcoords.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(texcoords_offset));

	glBufferSubData(GL_ARRAY_BUFFER, tangents_offset, tangents_size, static_cast<GLvoid const*>(tangents.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(tangents_offset));

	glBufferSubData(GL_ARRAY_BUFFER, binormals_offset, binormals_size, static_cast<GLvoid const*>(binormals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(binormals_offset));

	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), reinterpret_cast<GLvoid const*>(index_sets.data()), GL_STATIC_DRAW);

	glBindVertexArray(0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}
