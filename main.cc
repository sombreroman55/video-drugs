#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#include <cmath>

#define SCR_WIDTH 256
#define SCR_HEIGHT 256
#define LINE_TO_DEG (360.0f / SCR_HEIGHT)
#define PI (atan(1) / 4)

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

typedef enum {
    pastelle_rainbow,
    blue_greens,
    magicant,
    something,
} PaletteNumber;

const char* preset_palette_names[4] = {
    "Pastelle Rainbow",
    "Blue Greens",
    "Magicant",
    "i dunno"
};

uint32_t preset_palettes[4][16] = {
    { 0x051E3EFF, 0x251E3EFF, 0x451E3EFF, 0x651E3EFF, 0x851E3EFF, 0xFF3377FF, 0xFF5588FF, 0xFFBBEEFF, 0xFF99CCFF, 0xFF77AAFF, 0xFF8B94FF, 0xFFAAA5FF, 0xFFD3B6FF, 0xDCEDC1FF, 0xA8E6CFFF, 0x88D8B0FF },
    { 0x011F4BFF, 0x011F4BFF, 0x011F4BFF, 0x03396CFF, 0x03396CFF, 0x005B96FF, 0x6497B1FF, 0xB3CDE0FF, 0xB2D8D8FF, 0x66B2B2FF, 0x008080FF, 0x006666FF, 0x006666FF, 0x004C4CFF, 0x004C4CFF, 0x004C4CFF },
    { 0x9EDDFFFF, 0xA0CBFFFF, 0xA0C4FFFF, 0xC8B6FFFF, 0xD8BBFFFF, 0xE7BFFFFF, 0xFFC6FFFF, 0xFFCEFFFF, 0xFFD9FFFF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF },
    { 0xF72585FF, 0xB5179EFF, 0x7209B7FF, 0x560BADFF, 0x480CA8FF, 0x3A0CA3FF, 0x3F37C9FF, 0x4361EEFF, 0x4895EFFF, 0x4CC9F0FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF }
};

uint32_t uint_palette[16] = { 0 };
float float_palette[16][4] = { 0 };
uint8_t pixel_idxs[SCR_HEIGHT][SCR_WIDTH] = { 0 };
uint32_t pixels[SCR_WIDTH * SCR_HEIGHT] = { 0 };

void vertical_stripes(uint8_t* pixel_data, int h, int w, int colors)
{
    int i, j;
    for (i = 0; i < h; i++)
        for (j = 0; j < w; j++)
            pixel_data[i * h + j] = i / colors;
}

void vertical_stripes2(uint8_t* pixel_data, int h, int w, int colors)
{
    int i, j;
    for (i = 0; i < h; i++)
        for (j = 0; j < w; j++)
            pixel_data[i * h + j] = (i + j) % colors;
}

void horizontal_stripes(uint8_t* pixel_data, int h, int w, int colors)
{
    int i, j;
    for (j = 0; j < w; j++)
        for (i = 0; i < h; i++)
            pixel_data[i * h + j] = j / colors;
}

void concentric_squares(uint8_t* pixel_data, int h, int w, int colors)
{
    int chunk = colors * 2;
    int i, j, k, h_off = 0, w_off = 0, h_inc = h / chunk, w_inc = w / chunk;
    for (k = 0; k < colors; k++) {
        for (i = h_off; i < h - h_off; i++) {
            for (j = w_off; j < w - w_off; j++) {
                pixel_data[i * h + j] = k;
            }
        }
        h_off += h_inc;
        w_off += w_inc;
    }
}

static void update_palette(void)
{
    for (int i = 0; i < 16; i++) {
        uint8_t r = float_palette[i][0] * 255;
        uint8_t g = float_palette[i][1] * 255;
        uint8_t b = float_palette[i][2] * 255;
        uint8_t a = float_palette[i][3] * 255;
        uint_palette[i] = (a << 24) | (b << 16) | (g << 8) | r;
    }
}

static void use_preset_palette(PaletteNumber pn)
{
    memcpy(uint_palette, preset_palettes[pn], sizeof(uint_palette));
    for (int i = 0; i < 16; i++) {
        float_palette[i][0] = (uint_palette[i] >> 24 & 0xFF) / 255.0f;
        float_palette[i][1] = (uint_palette[i] >> 16 & 0xFF) / 255.0f;
        float_palette[i][2] = (uint_palette[i] >> 8 & 0xFF) / 255.0f;
        float_palette[i][3] = (uint_palette[i] >> 0 & 0xFF) / 255.0f;
    }
}

static void init_stuff(void)
{
    use_preset_palette(pastelle_rainbow);
    concentric_squares((uint8_t*)pixel_idxs, SCR_HEIGHT, SCR_WIDTH, 16);
}

int offset_from_angle(float deg, float amp, float freq, float xshift)
{
    freq /= 2 * PI;
    float rad = (deg * PI) / 180.0;
    return (int)(amp * sin(freq * rad + xshift));
}

// Main code
int main(int, char**)
{
    init_stuff();

    // Setup SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    Uint32 window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
    SDL_Window* window = SDL_CreateWindow("Video Drugs", 1280, 720, window_flags);
    if (window == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr) {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    SDL_ShowWindow(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    uint32_t frame_counter = 0;

    // Main loop
    bool done = false;
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!done)
#endif
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        static bool show_video_drugs = true;

        static bool palette_cycling = false;
        static int palette_cycle_interval = 3;

        static bool background_scrolling = false;
        static int xstep = 0;
        static int ystep = 0;

        static bool horizontal_oscillation = false;
        static bool interleaved_oscillation = false;
        static int interleave_interval = 1;
        static int hosc_interval = 1;
        static float hosc_amplitude = 1.0f;
        static float hosc_freq = 1.0f;
        static float hosc_xshift = 0.0f;

        static bool vertical_oscillation = false;
        static int vosc_interval = 1;
        static float vosc_amplitude = 1.0f;
        static float vosc_freq = 1.0f;
        static float vosc_xshift = 0.0f;

        static bool transparency = false;

        ImGui::Begin("Effects");
        {
            for (int i = 0; i < 16; i++) {
                char label[64];
                sprintf(label, "Palette Color %d", i + 1);
                ImGui::ColorEdit4(label, float_palette[i]);
            }
            ImGui::Checkbox("Palette Cycling", &palette_cycling);
            if (palette_cycling) {
                ImGui::SliderInt("Palette Cycle interval", &palette_cycle_interval, 1, 60);
            }

            ImGui::Checkbox("Background Scrolling", &background_scrolling);
            if (background_scrolling) {
                ImGui::SliderInt("X Scroll Step", &xstep, -10, 10);
                ImGui::SliderInt("Y Scroll Step", &ystep, -10, 10);
            }
            ImGui::Checkbox("Horizontal Oscillation", &horizontal_oscillation);
            if (horizontal_oscillation) {
                ImGui::Checkbox("Interleaved Oscillation", &interleaved_oscillation);
                if (interleaved_oscillation) {
                    ImGui::SliderInt("Interleave interval", &interleave_interval, 1, 20);
                }
                ImGui::SliderInt("H. Oscillation Interval", &hosc_interval, 1, 20);
                ImGui::SliderFloat("H. Oscillation Amplitude", &hosc_amplitude, 1, 1000);
                ImGui::SliderFloat("H. Oscillation Frequency", &hosc_freq, 1, 100);
                ImGui::SliderFloat("H. Oscillation Shift Increment", &hosc_xshift, 0, 1);
            }
            ImGui::Checkbox("Vertical Oscillation", &vertical_oscillation);
            if (vertical_oscillation) {
                ImGui::SliderInt("V. Oscillation Interval", &vosc_interval, 1, 20);
                ImGui::SliderFloat("V. Oscillation Amplitude", &vosc_amplitude, 1, 1000);
                ImGui::SliderFloat("V. Oscillation Frequency", &vosc_freq, 0, 100);
                ImGui::SliderFloat("V. Oscillation Shift Increment", &vosc_xshift, 0, 1);
            }
            // ImGui::Checkbox("Transparency", &transparency);

            update_palette();

            static int p_offset = 0;

            int interleave_mult = 1;
            int h_sine_offset = 0;
            static int h_scroll_offset = 0;
            static float h_sine_shift = 0.0f;

            int v_sine_offset = 0;
            static int v_scroll_offset = 0;
            static float v_sine_shift = 0.0f;
            if (palette_cycling) {
                if (frame_counter % (palette_cycle_interval + 1) == 0) {
                    p_offset = (p_offset + 1) % 16;
                }
            } else {
                p_offset = 0;
            }

            if (background_scrolling) {
                v_scroll_offset += SCR_HEIGHT + ystep;
                h_scroll_offset += SCR_WIDTH + xstep;
                v_scroll_offset %= SCR_HEIGHT;
                h_scroll_offset %= SCR_WIDTH;
            } else {
                h_scroll_offset = 0;
                v_scroll_offset = 0;
            }

            if (horizontal_oscillation) {
                if (frame_counter % (hosc_interval + 1) == 0) {
                    h_sine_shift += hosc_xshift;
                }
            }

            if (vertical_oscillation) {
                if (frame_counter % (vosc_interval + 1) == 0) {
                    v_sine_shift += vosc_xshift;
                }
            }

            int i, j;
            for (i = 0; i < SCR_HEIGHT; i++) {
                if (interleaved_oscillation) {
                    if (i % (interleave_interval) == 0) {
                        interleave_mult *= -1;
                    }
                } else {
                    interleave_mult = 1;
                }
                for (j = 0; j < SCR_WIDTH; j++) {
                    if (horizontal_oscillation) {
                        h_sine_offset = offset_from_angle(i * LINE_TO_DEG, hosc_amplitude, hosc_freq, h_sine_shift) * interleave_mult;
                    }
                    if (vertical_oscillation) {
                        v_sine_offset = offset_from_angle(i * LINE_TO_DEG, vosc_amplitude, vosc_freq, v_sine_shift);
                    }
                    int y = i + v_sine_offset + v_scroll_offset;
                    while (y < 0)
                        y += SCR_HEIGHT;
                    y %= SCR_HEIGHT;
                    int x = j + h_sine_offset + h_scroll_offset;
                    while (x < 0)
                        x += SCR_WIDTH;
                    x %= SCR_WIDTH;
                    int palette_index = pixel_idxs[y][x];
                    palette_index = (palette_index + p_offset) % 16;
                    pixels[i * SCR_WIDTH + j] = uint_palette[palette_index];
                }
            }
        }
        ImGui::End();
        ImGui::Begin("Image");
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            ImGui::Image((ImTextureID)(intptr_t)tex, ImVec2(SCR_WIDTH, SCR_HEIGHT));
        }
        ImGui::End();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
        frame_counter++;
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
