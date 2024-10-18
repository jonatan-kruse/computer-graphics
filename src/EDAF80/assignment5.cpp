// High score is 61
#include "assignment5.hpp"
#include "interpolation.hpp"
#include "parametric_shapes.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/ShaderProgramManager.hpp"
#include "core/node.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <tinyfiledialogs.h>

#include <iostream>
#include <unistd.h>

#include <clocale>
#include <cstdlib>
#include <stdexcept>

glm::vec3 random_pos_in_cube(float size) {
    return glm::vec3(
        size * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f),
        size * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f),
        size * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f));
}

edaf80::Assignment5::Assignment5(WindowManager &windowManager)
    : mCamera(
          0.5f * glm::half_pi<float>(),
          static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
          0.01f, 1000.0f),
      inputHandler(), mWindowManager(windowManager), window(nullptr) {
    WindowManager::WindowDatum window_datum{inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

    window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 5", window_datum, config::msaa_rate);
    if (window == nullptr) {
        throw std::runtime_error("Failed to get a window: aborting!");
    }

    bonobo::init();
}

edaf80::Assignment5::~Assignment5() {
    bonobo::deinit();
}

void edaf80::Assignment5::run() {
    auto sand_radius = 2.0f;
    auto gold_radius = 1.0f;
    auto player_radius = 1.5f;
    auto num_sand_spheres = 100;
    auto num_gold_spheres = 10;
    auto starting_speed = 5.0f;
    auto sand_cube_size = 75.0f;
    auto gold_cube_size = 50.0f;

    mCamera.mMovementSpeed = glm::vec3(starting_speed);

    GLuint skybox_texture = bonobo::loadTextureCubeMap("../../../res/cubemaps/Space/posx.png", "../../../res/cubemaps/Space/negx.png",
                                                       "../../../res/cubemaps/Space/posy.png", "../../../res/cubemaps/Space/negy.png",
                                                       "../../../res/cubemaps/Space/posz.png", "../../../res/cubemaps/Space/negz.png",
                                                       true);

    GLuint gold_sphere_diffuse_texture = bonobo::loadTexture2D("../../../res/textures/gold_color.jpg");
    GLuint gold_sphere_specular_texture = bonobo::loadTexture2D("../../../res/textures/gold_rough.jpg");
    GLuint gold_sphere_normal_texture = bonobo::loadTexture2D("../../../res/textures/gold_normal.png");

    GLuint player_diffuse_texture = bonobo::loadTexture2D("../../../res/textures/ship_color.jpg");
    GLuint player_specular_texture = bonobo::loadTexture2D("../../../res/textures/ship_rough.jpg");
    GLuint player_normal_texture = bonobo::loadTexture2D("../../../res/textures/ship_normal.png");

    GLuint sand_sphere_diffuse_texture = bonobo::loadTexture2D("../../../res/textures/sand_color.jpg");
    GLuint sand_sphere_specular_texture = bonobo::loadTexture2D("../../../res/textures/sand_rough.jpg");
    GLuint sand_sphere_normal_texture = bonobo::loadTexture2D("../../../res/textures/sand_normal.jpg");

    // Create the shader programs
    ShaderProgramManager program_manager;
    GLuint skybox_shader = 0u;
    program_manager.CreateAndRegisterProgram("Skybox",
                                             {{ShaderType::vertex, "EDAF80/skybox.vert"},
                                              {ShaderType::fragment, "EDAF80/skybox.frag"}},
                                             skybox_shader);
    if (skybox_shader == 0u) {
        LogError("Failed to load skybox shader");
        return;
    }

    GLuint phong_shader = 0u;
    program_manager.CreateAndRegisterProgram("Phong",
                                             {{ShaderType::vertex, "EDAF80/phong.vert"},
                                              {ShaderType::fragment, "EDAF80/phong.frag"}},
                                             phong_shader);
    if (phong_shader == 0u) {
        LogError("Failed to load phong shader");
        return;
    }

    auto light_position = glm::vec3(0.0f, 100.0f, 50.0f);
    auto const set_uniforms = [&light_position](GLuint program) {
        glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
    };

    auto camera_position = mCamera.mWorld.GetTranslation();

    bonobo::material_data sand_material;
    sand_material.ambient = glm::vec3(80.0f / 255.0f, 55.0f / 255.0f, 90.0f / 255.0f);
    sand_material.diffuse = glm::vec3(220.0f / 255.0f, 220.0f / 255.0f, 220.0f / 255.0f);
    sand_material.specular = glm::vec3(255.0f / 255.0f, 235.0f / 255.0f, 255.0f / 255.0f);

    auto const sand_phong_set_uniforms = [&light_position, &camera_position, &sand_material](GLuint program) {
        glUniform1i(glGetUniformLocation(program, "use_normal_mapping"), 1);
        glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
        glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
        glUniform3fv(glGetUniformLocation(program, "ambient"), 1, glm::value_ptr(sand_material.ambient));
        glUniform3fv(glGetUniformLocation(program, "diffuse"), 1, glm::value_ptr(sand_material.diffuse));
        glUniform3fv(glGetUniformLocation(program, "specular"), 1, glm::value_ptr(sand_material.specular));
        glUniform1f(glGetUniformLocation(program, "shininess"), 10.0f);
    };

    bonobo::material_data gold_material;
    gold_material.ambient = glm::vec3(80.0f / 255.0f, 45.0f / 255.0f, 90.0f / 255.0f);
    gold_material.diffuse = glm::vec3(220.0f / 255.0f, 220.0f / 255.0f, 220.0f / 255.0f);
    gold_material.specular = glm::vec3(255.0f / 255.0f, 200.0f / 255.0f, 255.0f / 255.0f);
    gold_material.shininess = 10.0f;

    auto const gold_phong_set_uniforms = [&light_position, &camera_position, &gold_material](GLuint program) {
        glUniform1i(glGetUniformLocation(program, "use_normal_mapping"), 1);
        glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
        glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
        glUniform3fv(glGetUniformLocation(program, "ambient"), 1, glm::value_ptr(gold_material.ambient));
        glUniform3fv(glGetUniformLocation(program, "diffuse"), 1, glm::value_ptr(gold_material.diffuse));
        glUniform3fv(glGetUniformLocation(program, "specular"), 1, glm::value_ptr(gold_material.specular));
        glUniform1f(glGetUniformLocation(program, "shininess"), gold_material.shininess);
    };

    bonobo::material_data player_material;
    player_material.ambient = glm::vec3(160.0f / 255.0f, 130.0f / 255.0f, 150.0f / 255.0f);
    player_material.diffuse = glm::vec3(220.0f / 255.0f, 220.0f / 255.0f, 220.0f / 255.0f);
    player_material.specular = glm::vec3(255.0f / 255.0f, 250.0f / 255.0f, 255.0f / 255.0f);
    player_material.shininess = 10.0f;

    auto const player_phong_set_uniforms = [&light_position, &camera_position, &player_material](GLuint program) {
        glUniform1i(glGetUniformLocation(program, "use_normal_mapping"), 1);
        glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
        glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
        glUniform3fv(glGetUniformLocation(program, "ambient"), 1, glm::value_ptr(player_material.ambient));
        glUniform3fv(glGetUniformLocation(program, "diffuse"), 1, glm::value_ptr(player_material.diffuse));
        glUniform3fv(glGetUniformLocation(program, "specular"), 1, glm::value_ptr(player_material.specular));
        glUniform1f(glGetUniformLocation(program, "shininess"), player_material.shininess);
    };

    auto skybox_shape = parametric_shapes::createSphere(100.0f, 100u, 100u);
    if (skybox_shape.vao == 0u) {
        LogError("Failed to retrieve the mesh for the skybox");
        return;
    }

    Node skybox;
    skybox.set_geometry(skybox_shape);
    skybox.set_program(&skybox_shader, set_uniforms);
    skybox.add_texture("skybox", skybox_texture, GL_TEXTURE_CUBE_MAP);

    auto sand_shapes = new std::vector<bonobo::mesh_data>();

    for (auto i = 0; i < num_sand_spheres; i++) {
        auto sand_shape = parametric_shapes::createSphere(sand_radius, 100u, 100u);
        if (sand_shape.vao == 0u) {
            LogError("Failed to retrieve the mesh for the sand sphere");
            return;
        }

        sand_shapes->push_back(sand_shape);
    }

    auto sand_nodes = new std::vector<Node>();
    auto sand_nodes_positions = new std::vector<glm::vec3>();

    for (auto i = 0; i < num_sand_spheres; i++) {
        Node sand_sphere;
        sand_sphere.set_geometry(sand_shapes->at(i));
        sand_sphere.set_material_constants(gold_material);
        sand_sphere.set_program(&phong_shader, sand_phong_set_uniforms);
        sand_sphere.add_texture("diffuseMap", sand_sphere_diffuse_texture, GL_TEXTURE_2D);
        sand_sphere.add_texture("specularMap", sand_sphere_specular_texture, GL_TEXTURE_2D);
        sand_sphere.add_texture("normalMap", sand_sphere_normal_texture, GL_TEXTURE_2D);
        auto position = random_pos_in_cube(sand_cube_size);
        // while the sand node is colliding with another sand node we need to change the position
        while (true) {
            bool collision = false;
            for (auto &sand_node_position : *sand_nodes_positions) {
                if (glm::distance(position, sand_node_position) < 2 * sand_radius) {
                    collision = true;
                    break;
                }
            }
            if (!collision) {
                break;
            }
            position = random_pos_in_cube(sand_cube_size);
        }
        sand_sphere.get_transform().SetTranslate(position);

        sand_nodes_positions->push_back(position);
        sand_nodes->push_back(sand_sphere);
    }

    auto gold_shapes = new std::vector<bonobo::mesh_data>();

    for (auto i = 0; i < num_gold_spheres; i++) {
        auto gold_shape = parametric_shapes::createSphere(gold_radius, 100u, 100u);
        if (gold_shape.vao == 0u) {
            LogError("Failed to retrieve the mesh for the gold sphere");
            return;
        }

        gold_shapes->push_back(gold_shape);
    }

    auto gold_nodes = new std::vector<Node>();
    auto gold_nodes_positions = new std::vector<glm::vec3>();

    for (auto i = 0; i < num_gold_spheres; i++) {
        Node gold_sphere;
        gold_sphere.set_geometry(gold_shapes->at(i));
        gold_sphere.set_material_constants(gold_material);
        gold_sphere.set_program(&phong_shader, gold_phong_set_uniforms);
        gold_sphere.add_texture("diffuseMap", gold_sphere_diffuse_texture, GL_TEXTURE_2D);
        gold_sphere.add_texture("specularMap", gold_sphere_specular_texture, GL_TEXTURE_2D);
        gold_sphere.add_texture("normalMap", gold_sphere_normal_texture, GL_TEXTURE_2D);
        auto position = random_pos_in_cube(gold_cube_size);
        // while the gold node is colliding with a sand node we need to change the position
        while (true) {
            bool collision = false;
            for (auto &sand_node_position : *sand_nodes_positions) {
                if (glm::distance(position, sand_node_position) < 2 * sand_radius) {
                    collision = true;
                    break;
                }
            }
            if (!collision) {
                break;
            }
            position = random_pos_in_cube(gold_cube_size);
        }
        gold_sphere.get_transform().SetTranslate(position);

        gold_nodes_positions->push_back(position);
        gold_nodes->push_back(gold_sphere);
    }

    auto player_shape = parametric_shapes::createSpaceShip();
    if (player_shape.vao == 0u) {
        LogError("Failed to retrieve the mesh for the player");
        return;
    }

    Node player;
    player.set_geometry(player_shape);
    player.set_material_constants(player_material);
    player.set_program(&phong_shader, player_phong_set_uniforms);
    player.add_texture("diffuseMap", player_diffuse_texture, GL_TEXTURE_2D);
    player.add_texture("specularMap", player_specular_texture, GL_TEXTURE_2D);
    player.add_texture("normalMap", player_normal_texture, GL_TEXTURE_2D);

    glClearDepthf(1.0f);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    auto lastTime = std::chrono::high_resolution_clock::now();

    bool shader_reload_failed = false;

    while (!glfwWindowShouldClose(window)) {
        auto const nowTime = std::chrono::high_resolution_clock::now();
        auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
        lastTime = nowTime;

        auto &io = ImGui::GetIO();
        inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

        glfwPollEvents();
        inputHandler.Advance();
        mCamera.UpdateGame(deltaTimeUs, inputHandler, player.get_transform());

        camera_position = mCamera.mWorld.GetTranslation();

        if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
            shader_reload_failed = !program_manager.ReloadAllPrograms();
            if (shader_reload_failed)
                tinyfd_notifyPopup("Shader Program Reload Error",
                                   "An error occurred while reloading shader programs; see the logs for details.\n"
                                   "Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
                                   "error");
        }
        if (inputHandler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
            mWindowManager.ToggleFullscreenStatusForWindow(window);

        int framebuffer_width, framebuffer_height;
        glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
        glViewport(0, 0, framebuffer_width, framebuffer_height);

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        skybox.render(mCamera.GetWorldToClipMatrix());
        skybox.get_transform().SetTranslate(mCamera.mWorld.GetTranslation());
        for (auto &gold_node : *gold_nodes) {
            gold_node.render(mCamera.GetWorldToClipMatrix());
        }
        for (auto &sand_node : *sand_nodes) {
            sand_node.render(mCamera.GetWorldToClipMatrix());
        }
        player.render(mCamera.GetWorldToClipMatrix());

        // if the player is colliding with a gold node we need to change the position of the gold node and add speed to the player
        for (auto i = 0; i < num_gold_spheres; i++) {
            if (glm::distance(player.get_transform().GetTranslation(), gold_nodes->at(i).get_transform().GetTranslation()) < gold_radius + player_radius) {
                auto position = random_pos_in_cube(gold_cube_size);
                // make sure the new position is not colliding with a sand node
                while (true) {
                    bool collision = false;
                    for (auto &sand_node_position : *sand_nodes_positions) {
                        if (glm::distance(position, sand_node_position) < 2 * sand_radius) {
                            collision = true;
                            break;
                        }
                    }
                    if (!collision) {
                        break;
                    }
                    position = random_pos_in_cube(gold_cube_size);
                }
                gold_nodes->at(i).get_transform().SetTranslate(position);
                gold_nodes_positions->at(i) = position;
                mCamera.mMovementSpeed += glm::vec3(0.5f);
                std::cout << "Score: " << (mCamera.mMovementSpeed.x - starting_speed) * 2.0f << std::endl;
            }
        }

        // if the player is colliding with a sand node we need to change the position of the sand node and remove speed from the player
        for (auto i = 0; i < num_sand_spheres; i++) {
            if (glm::distance(player.get_transform().GetTranslation(), sand_nodes->at(i).get_transform().GetTranslation()) < sand_radius + player_radius) {
                auto position = random_pos_in_cube(sand_cube_size);
                // make sure the new position is not colliding with a gold node
                while (true) {
                    bool collision = false;
                    for (auto &gold_node_position : *gold_nodes_positions) {
                        if (glm::distance(position, gold_node_position) < 2 * gold_radius) {
                            collision = true;
                            break;
                        }
                    }
                    if (!collision) {
                        break;
                    }
                    position = random_pos_in_cube(sand_cube_size);
                }
                sand_nodes->at(i).get_transform().SetTranslate(position);
                sand_nodes_positions->at(i) = position;
                std::cout << "You lost!" << std::endl;
                std::cout << "Your score was: " << (mCamera.mMovementSpeed.x - starting_speed) * 2.0f << std::endl;
                mCamera.mMovementSpeed = glm::vec3(starting_speed);
            }
        }

        glfwSwapBuffers(window);
    }
}

int main() {
    std::setlocale(LC_ALL, "");

    Bonobo framework;

    try {
        edaf80::Assignment5 assignment5(framework.GetWindowManager());
        assignment5.run();
    } catch (std::runtime_error const &e) {
        LogError(e.what());
    }
}