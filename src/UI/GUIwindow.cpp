/*
 *  Created on: Mar 26, 2021
 *      Author: yankai
 */
#ifdef USE_GUI
#include "GUIwindow.h"
#include "../Script/Kiss.h"

#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include "../../GUI/nuklear.h"
#include "../../GUI/backend/glfw_opengl3/nuklear_glfw_gl3.h"

namespace kai
{

    GUIwindow::GUIwindow()
    {
    }

    GUIwindow::~GUIwindow()
    {
    }

    bool GUIwindow::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        return true;
    }

    bool GUIwindow::setup(void)
    {
        /* Platform */
        struct nk_glfw glfw = {0};
        static GLFWwindow *win;
        int width = 0, height = 0;
        struct nk_context *ctx;
        struct nk_colorf bg;

        /* GLFW */
        glfwSetErrorCallback(error_callback);
        if (!glfwInit())
        {
            fprintf(stdout, "[GFLW] failed to init!\n");
            exit(1);
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Demo", NULL, NULL);
        glfwMakeContextCurrent(win);
        glfwGetWindowSize(win, &width, &height);

        /* OpenGL */
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glewExperimental = 1;
        if (glewInit() != GLEW_OK)
        {
            fprintf(stderr, "Failed to setup GLEW\n");
            exit(1);
        }

        ctx = nk_glfw3_init(&glfw, win, NK_GLFW3_INSTALL_CALLBACKS);
        /* Load Fonts: if none of these are loaded a default font will be used  */
        /* Load Cursor: if you uncomment cursor loading please hide the cursor */
        {
            struct nk_font_atlas *atlas;
            nk_glfw3_font_stash_begin(&glfw, &atlas);
            /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
            /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 14, 0);*/
            /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
            /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
            /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
            /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
            nk_glfw3_font_stash_end(&glfw);
        /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
    /*nk_style_set_font(ctx, &droid->handle);*/}

#ifdef INCLUDE_STYLE
/*set_style(ctx, THEME_WHITE);*/
/*set_style(ctx, THEME_RED);*/
/*set_style(ctx, THEME_BLUE);*/
/*set_style(ctx, THEME_DARK);*/
#endif

        bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
        while (!glfwWindowShouldClose(win))
        {
            /* Input */
            glfwPollEvents();
            nk_glfw3_new_frame(&glfw);

            /* GUI */
            if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
                         NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                             NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
            {
                enum
                {
                    EASY,
                    HARD
                };
                static int op = EASY;
                static int property = 20;
                nk_layout_row_static(ctx, 30, 80, 1);
                if (nk_button_label(ctx, "button"))
                    fprintf(stdout, "button pressed\n");

                nk_layout_row_dynamic(ctx, 30, 2);
                if (nk_option_label(ctx, "easy", op == EASY))
                    op = EASY;
                if (nk_option_label(ctx, "hard", op == HARD))
                    op = HARD;

                nk_layout_row_dynamic(ctx, 25, 1);
                nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, "background:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 25, 1);
                if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx), 400)))
                {
                    nk_layout_row_dynamic(ctx, 120, 1);
                    bg = nk_color_picker(ctx, bg, NK_RGBA);
                    nk_layout_row_dynamic(ctx, 25, 1);
                    bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f, 0.005f);
                    bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f, 0.005f);
                    bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f, 0.005f);
                    bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f, 0.005f);
                    nk_combo_end(ctx);
                }
            }
            nk_end(ctx);

/* -------------- EXAMPLES ---------------- */
#ifdef INCLUDE_CALCULATOR
            calculator(ctx);
#endif
#ifdef INCLUDE_OVERVIEW
            overview(ctx);
#endif
#ifdef INCLUDE_NODE_EDITOR
            node_editor(ctx);
#endif
            /* ----------------------------------------- */

            /* Draw */
            glfwGetWindowSize(win, &width, &height);
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT);
            glClearColor(bg.r, bg.g, bg.b, bg.a);
            /* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
         * with blending, scissor, face culling, depth test and viewport and
         * defaults everything back into a default state.
         * Make sure to either a.) save and restore or b.) reset your own state after
         * rendering the UI. */
            nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
            glfwSwapBuffers(win);
        }
        nk_glfw3_shutdown(&glfw);
        glfwTerminate();
//        return 0;

        return true;
    }

    int GUIwindow::check(void)
    {
        return this->_ModuleBase::check();
    }

    bool GUIwindow::start ( void )
    {
        NULL_F ( m_pT );
        return m_pT->start ( getUpdate, this );
    }

    void GUIwindow::update ( void )
    {
        sleep(5);
        setup();

        while ( m_pT->bRun() )
        {
            m_pT->autoFPSfrom();

            updateGUI();

            m_pT->autoFPSto();
        }
    }

    bool GUIwindow::updateGUI ( void )
    {
        return true;
    }

    // void GUIwindow::CbRecvData ( LivoxEthPacket* pData, void* pLivox )
    // {
    // }

    void GUIwindow::draw(void)
    {
        this->_ModuleBase::draw();
    }

}
#endif
