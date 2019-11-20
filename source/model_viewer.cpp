#include "common.h"
#include "SourcePath.h"


using namespace Angel;

typedef vec4 color4;



//
//vec4 light_position(   0.0, 0.0, 1.0, 0.0 );
//color4 light_ambient(  0.1, 0.1, 0.1, 1.0 );
//color4 light_diffuse(  1.0, 1.0, 1.0, 1.0 );
//color4 light_specular( 1.0, 1.0, 1.0, 1.0 );
//
//// Initialize shader material parameters
//color4 material_ambient( 0.1, 0.1, 0.1, 1.0 );
//color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
//color4 material_specular( 0.8, 0.8, 0.8, 1.0 );
//float  material_shininess = 10;

enum{_CUBE, _TEAPOT, _BUNNY, _DRAGON, _BUDDHA, _BB8, _R2D2, _TIE_FIGHTER, _TOTAL_MODELS};
std::string files[_TOTAL_MODELS] = {"/models/armadillo.obj",
    "/models/wt_teapot.obj",
    "/models/bunny.obj",
    "/models/dragon.obj",
    "/models/buddha.obj",
    "/models/BB8.obj",
    "/models/R2D2_Standing.obj",
    "/models/Tie_Fighter.obj"};
namespace GLState {
    int window_width, window_height;
    
    bool render_line;
    
    std::vector < GLuint > vao;
    std::vector < GLuint > buffer;
    std::vector < Mesh > mesh;
    GLuint vPosition, vNormal, vTexCoord;
    
    GLuint program;
    
    // Model-view and projection matrices uniform location
    GLuint  ModelView, ModelViewLight, NormalMatrix, Projection;
    GLuint ModelView_loc, Projection_loc, NormalMatrix_loc;

    //==========Trackball Variables==========
    static float curquat[4],lastquat[4];
    /* current transformation matrix */
    static float curmat[4][4];
    mat4 curmat_a;
    /* actual operation  */
    static int scaling;
    static int moving;
    static int panning;
    /* starting "moving" coordinates */
    static int beginx, beginy;
    /* ortho */
    float ortho_x, ortho_y;
    /* current scale factor */
    static float scalefactor;
    bool lbutton_down;
    mat4  projection;
    mat4 user_MV;
    float mousex;
    float mousey;
//        mat4 sceneModelView;
    
    color4 light_ambient;
    color4 light_diffuse;
    color4 light_specular;
    
//    bool scaling;
//    bool moving;
//    bool panning;
    
};
//
//std::vector < Mesh > mesh;
//std::vector < GLuint > buffer;
//std::vector < GLuint > vao;
//GLuint ModelView_loc, NormalMatrix_loc, Projection_loc;
bool wireframe;
int current_draw;
//
////==========Trackball Variables==========
//static float curquat[4],lastquat[4];
///* current transformation matrix */
//static float curmat[4][4];
//mat4 curmat_a;
///* actual operation  */
//bool scaling;
//bool moving;
//bool panning;
///* starting "moving" coordinates */
//static int beginx, beginy;
///* ortho */
//float ortho_x, ortho_y;
///* current scale factor */
//static float scalefactor;
//bool lbutton_down;


static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}


//User interaction handler
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
        current_draw = (current_draw+1)%_TOTAL_MODELS;
    }
    if (key == GLFW_KEY_W && action == GLFW_PRESS){
        wireframe = !wireframe;
    }
}

//User interaction handler
static void mouse_click(GLFWwindow* window, int button, int action, int mods){
    
    if (GLFW_RELEASE == action){
        GLState::moving=GLState::scaling=GLState::panning=false;
        return;
    }
    
    if( mods & GLFW_MOD_SHIFT){
        GLState::scaling=true;
    }else if( mods & GLFW_MOD_ALT ){
        GLState::panning=true;
    }else{
        GLState::moving=true;
        trackball(GLState::lastquat, 0, 0, 0, 0);
    }
    
    double xpos, ypos;

    glfwGetCursorPos(window, &xpos, &ypos);
    GLState::beginx = xpos; GLState::beginy = ypos;

}



//User interaction handler
void mouse_move(GLFWwindow* window, double x, double y){
    
    int W, H;
    glfwGetFramebufferSize(window, &W, &H);
        double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    GLState::mousex = xpos;
    GLState::mousey = ypos;
    std::cout << xpos;
    std::cout << " \n";
    std::cout << ypos;
    std::cout << " \n";
    
    float dx=(x-GLState::beginx)/(float)W;
    float dy=(GLState::beginy-y)/(float)H;
    
    if (GLState::panning)
    {
        GLState::ortho_x  +=dx;
        GLState::ortho_y  +=dy;
        
        GLState::beginx = x; GLState::beginy = y;
        return;
    }
    else if (GLState::scaling)
    {
        GLState::scalefactor *= (1.0f+dx);
        
        GLState::beginx = x;
        GLState::beginy = y;
        return;
    }
    else if (GLState::moving)
    {
        trackball(GLState::lastquat,
                  (2.0f * GLState::beginx - W) / W,
                  (H - 2.0f * GLState::beginy) / H,
                  (2.0f * x - W) / W,
                  (H - 2.0f * y) / H
                  );
        
        add_quats(GLState::lastquat, GLState::curquat, GLState::curquat);
        build_rotmatrix(GLState::curmat, GLState::curquat);
        
        GLState::beginx = x;GLState::beginy = y;
        return;
    }
}

float lerp(float Input, float InputLow, float InputHigh, float OutputLow, float OutputHigh ){
    return ((Input - InputLow) / (InputHigh - InputLow)) * (OutputHigh - OutputLow) + OutputLow;
}
void init(){
    



    std::string vshader = source_path + "/shaders/vshader.glsl";
    std::string fshader = source_path + "/shaders/fshader.glsl";
    
    GLchar* vertex_shader_source = readShaderSource(vshader.c_str());
    GLchar* fragment_shader_source = readShaderSource(fshader.c_str());
    
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const GLchar**) &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    check_shader_compilation(vshader, vertex_shader);
    
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const GLchar**) &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    check_shader_compilation(fshader, fragment_shader);
    
    GLState::program = glCreateProgram();
    glAttachShader(GLState::program, vertex_shader);
    glAttachShader(GLState::program, fragment_shader);
    
    glLinkProgram(GLState::program);
    check_program_link(GLState::program);
    
    glUseProgram(GLState::program);
    
    glBindFragDataLocation(GLState::program, 0, "fragColor");
    
    //Per vertex attributes
    GLState::vPosition = glGetAttribLocation( GLState::program, "vPosition" );
    GLState::vNormal = glGetAttribLocation( GLState::program, "vNormal" );
    
//    //Compute ambient, diffuse, and specular terms
//    color4 ambient_product  = light_ambient * material_ambient;
//    color4 diffuse_product  = light_diffuse * material_diffuse;
//    color4 specular_product = light_specular * material_specular;
//
//    //Retrieve and set uniform variables
//    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, light_position);
//    glUniform4fv( glGetUniformLocation(program, "AmbientProduct"), 1, ambient_product );
//    glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"), 1, diffuse_product );
//    glUniform4fv( glGetUniformLocation(program, "SpecularProduct"), 1, specular_product );
//    glUniform1f(  glGetUniformLocation(program, "Shininess"), material_shininess );
    
    //Matrix uniform variable locations
    GLState::ModelView_loc = glGetUniformLocation( GLState::program, "ModelView" );
    GLState::NormalMatrix_loc = glGetUniformLocation( GLState::program, "NormalMatrix" );
    GLState::Projection_loc = glGetUniformLocation( GLState::program, "Projection" );
    
    //===== Send data to GPU ======
    GLState::vao.resize(_TOTAL_MODELS);
    glGenVertexArrays( _TOTAL_MODELS, &GLState::vao[0] );
    
    GLState::buffer.resize(_TOTAL_MODELS);
    glGenBuffers( _TOTAL_MODELS, &GLState::buffer[0] );
    
    for(unsigned int i=0; i < _TOTAL_MODELS; i++){
        GLState::mesh.push_back((source_path + files[i]).c_str());
        
        glBindVertexArray( GLState::vao[i] );
        glBindBuffer( GL_ARRAY_BUFFER, GLState::buffer[i] );
        unsigned int vertices_bytes = GLState::mesh[i].vertices.size()*sizeof(vec4);
        unsigned int normals_bytes  = GLState::mesh[i].normals.size()*sizeof(vec3);
        
        glBufferData( GL_ARRAY_BUFFER, vertices_bytes + normals_bytes, NULL, GL_STATIC_DRAW );
        unsigned int offset = 0;
        glBufferSubData( GL_ARRAY_BUFFER, offset, vertices_bytes, &GLState::mesh[i].vertices[0] );
        offset += vertices_bytes;
        glBufferSubData( GL_ARRAY_BUFFER, offset, normals_bytes,  &GLState::mesh[i].normals[0] );
        
        glEnableVertexAttribArray( GLState::vNormal );
        glEnableVertexAttribArray( GLState::vPosition );
        
        glVertexAttribPointer( GLState::vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
        glVertexAttribPointer( GLState::vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(GLState::mesh[i].vertices.size()*sizeof(vec4)) );
        
    }
    
    //===== End: Send data to GPU ======
    
    
    // ====== Enable some opengl capabilitions ======
    glEnable( GL_DEPTH_TEST );
    glShadeModel(GL_SMOOTH);
    
    glClearColor( 0.8, 0.8, 1.0, 1.0 );
    
    //===== Initalize some program state variables ======
    
    //Quaternion trackball variables, you can ignore
    GLState::scaling  = 0;
    GLState::moving   = 0;
    GLState::panning  = 0;
    GLState::beginx   = 0;
    GLState::beginy   = 0;
    
    matident(GLState::curmat);
    trackball(GLState::curquat , 0.0f, 0.0f, 0.0f, 0.0f);
    trackball(GLState::lastquat, 0.0f, 0.0f, 0.0f, 0.0f);
    add_quats(GLState::lastquat, GLState::curquat, GLState::curquat);
    build_rotmatrix(GLState::curmat, GLState::curquat);
    
    GLState::scalefactor = 1.0;
    
    wireframe = false;
    current_draw = 0;
    
    GLState::lbutton_down = false;
    
    
    //===== End: Initalize some program state variables ======
    
}


void drawObject(GLFWwindow* window){
    // Initialize shader material parameters
//    std::cout << lerp(sin(glfwGetTime()), -1., 1., 1., 150.);

//    vec4 light_position(   lerp(cos(glfwGetTime()), -1., 1., 0., 2.), lerp(sin(glfwGetTime()), -1., 1., 0., 2.), lerp(sin(glfwGetTime()), -1., 1., 0., 2.) , 0.0 );
//    color4 light_ambient(  0.1, 0.1, 0.1, 1.0 );
//    color4 light_diffuse(  1.0, 1.0, 1.0, 1.0 );
//    color4 light_specular( 1.0, 1.0, 1.0, 1.0 );
    
    vec4 light_position( lerp(GLState::mousex, -1000., 1000., -5., 5.),  lerp(GLState::mousey, -1000., 1000., -5., 5.), lerp(GLState::mousey, -1000., 1000., -4.5, 3.5) , 0.);
    color4 light_ambient(  0.1, 0.1, 0.1, 1.0 );
    color4 light_diffuse(  1.0, 1.0, 1.0, 1.0 );
    color4 light_specular( 1.0, 1.0, 1.0, 1.0 );
    
    
    
    // Initialize shader material parameters
    color4 material_ambient( 0.1, 0.1, 0.1, 1.0 );
//    color4 material_diffuse( abs(sin(glfwGetTime())), abs(cos(glfwGetTime())), 0.0, 1.0 );
    //color4 material_ambient( 0.1, 0.1, 0.1, 1.0 );
    color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
    color4 material_specular( 0.8, 0.8, 0.8, 1.0 );
    //float  material_shininess = 10;

    float  material_shininess = 10;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
//      glfwGetFramebufferSize(window, 50, 50);
    glViewport(0, 0, width, height);
    
    GLfloat aspect = GLfloat(width)/height;
    
    //Projection matrix
    mat4  projection = Perspective( 45.0, aspect, 0.5, 5.0 );
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //"Camera" position
    const vec3 viewer_pos( 0.0, 0.0, 2.0 );
    //Compute ambient, diffuse, and specular terms
    color4 ambient_product  = light_ambient * material_ambient;
    color4 diffuse_product  = light_diffuse * material_diffuse;
    color4 specular_product = light_specular * material_specular;
    

    //Track_ball rotation matrix
    mat4 track_ball =  mat4(GLState::curmat[0][0], GLState::curmat[1][0],
                            GLState::curmat[2][0], GLState::curmat[3][0],
                            GLState::curmat[0][1], GLState::curmat[1][1],
                            GLState::curmat[2][1], GLState::curmat[3][1],
                            GLState::curmat[0][2], GLState::curmat[1][2],
                            GLState::curmat[2][2], GLState::curmat[3][2],
                            GLState::curmat[0][3], GLState::curmat[1][3],
                            GLState::curmat[2][3], GLState::curmat[3][3]);
//    mat4 user_MV  =  Translate( -viewer_pos ) *                    //Move Camera Back to -viewer_pos
    GLState::user_MV  =  Translate(-viewer_pos) *   //Move Camera Back
    Translate(GLState::ortho_x, GLState::ortho_y, 0.0) *
    track_ball *                   //Rotate Camera
    Scale(GLState::scalefactor,
          GLState::scalefactor,
          GLState::scalefactor);   //User Scale
    
    // ====== Draw ======
    glBindVertexArray(GLState::vao[current_draw]);
    //glBindBuffer( GL_ARRAY_BUFFER, buffer[current_draw] );
    
//    glUniformMatrix4fv( ModelView_loc, 1, GL_TRUE, user_MV*mesh[current_draw].model_view);
//    glUniformMatrix4fv( Projection_loc, 1, GL_TRUE, projection );
//    glUniformMatrix4fv( NormalMatrix_loc, 1, GL_TRUE, transpose(invert(user_MV*mesh[current_draw].model_view)));
    //Retrieve and set uniform variables
    glUniform4fv( glGetUniformLocation(GLState::program, "LightPosition"), 1, light_position);
    glUniform4fv( glGetUniformLocation(GLState::program, "AmbientProduct"), 1, ambient_product  );
    glUniform4fv( glGetUniformLocation(GLState::program, "DiffuseProduct"), 1, diffuse_product );
    glUniform4fv( glGetUniformLocation(GLState::program, "SpecularProduct"), 1, specular_product );
    glUniform1f(  glGetUniformLocation(GLState::program, "Shininess"), material_shininess );
    glUniformMatrix4fv( GLState::ModelView_loc, 1, GL_TRUE, GLState::user_MV*GLState::mesh[current_draw].model_view);
    glUniformMatrix4fv( GLState::Projection_loc, 1, GL_TRUE, projection );
    glUniformMatrix4fv( GLState::NormalMatrix_loc, 1, GL_TRUE, transpose(invert(GLState::user_MV*GLState::mesh[current_draw].model_view)));
    
    glDrawArrays( GL_TRIANGLES, 0, GLState::mesh[current_draw].vertices.size() );
    // ====== End: Draw ======
}
    

int main(void){
    
    GLFWwindow* window;
    
    glfwSetErrorCallback(error_callback);
    
    if (!glfwInit())
        exit(EXIT_FAILURE);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    
    window = glfwCreateWindow(512, 512, "Assignment 3 - 3D Shading", NULL, NULL);
    if (!window){
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    //Set key and mouse callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_click);
    glfwSetCursorPosCallback(window, mouse_move);
    
    
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);
    
    init();
    
    while (!glfwWindowShouldClose(window)){
        
        //Display as wirfram, boolean tied to keystoke 'w'
        if(wireframe){
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        }else{
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        }
        

        
        drawObject(window);
        
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }
    
    glfwDestroyWindow(window);
    
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

