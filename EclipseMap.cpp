#include "EclipseMap.h"

using namespace std;

struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texture;

    vertex() {}

    vertex(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &texture) : position(position),
                                                                                           normal(normal),
                                                                                           texture(texture) {}
};

struct triangle {
    int vertex1;
    int vertex2;
    int vertex3;

    triangle() {}

    triangle(const int &vertex1, const int &vertex2, const int &vertex3) : vertex1(vertex1), vertex2(vertex2),
                                                                           vertex3(vertex3) {}
};

static bool isResized = false;
static int resizedWidth = 1000;
static int resizedHeight = 1000;

static void resizeEvent(GLFWwindow *wind, int width, int height){
    isResized = true;
    resizedHeight = height;
    resizedWidth = width;
}

void EclipseMap::Render(const char *coloredTexturePath, const char *greyTexturePath, const char *moonTexturePath) {
    // Open window
    GLFWwindow *window = openWindow(windowName, screenWidth, screenHeight);

    glfwSetFramebufferSizeCallback(window, resizeEvent);

    glm::mat4 MVP1,MVP, M_model, M_view, M_projection, inv_Model,M_model1,M_model2,M_model3;
    M_model = glm::mat4(1.0f);
    M_model1 = glm::mat4(1.0f);
    M_model2 = glm::mat4(1.0f);
    M_model3 = glm::mat4(1.0f);
    glm::mat4 I = glm::mat4(1.0f);
    int MVPLoc, ProjectionMatrixLoc, ViewMatrixLoc, heightFactorLoc, cameraPositionLoc, lightPositionLoc, textureOffsetLoc, orbitDegreeLoc, TexColorLoc, MoonTexColorLoc, TexGreyLoc, NormalMatrixLoc, ModelMatrixLoc;
    // Moon commands
    // Load shaders
    GLuint moonShaderID = initShaders("moonShader.vert", "moonShader.frag");

    initMoonColoredTexture(moonTexturePath, moonShaderID);

    // TODO: Set moonVertices *****************************************************
    vector<vertex> moonVertices;
    vector<vertex> worldVertices;

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / moonRadius;    // vertex normal
    float s, t;                                     // vertex texCoord
    float sectorStep = 2 * M_PI / horizontalSplitCount;
    float stackStep = M_PI / verticalSplitCount;
    float sectorAngle, stackAngle;

    for(int i = 0 ; i<= verticalSplitCount;++i){
        stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = moonRadius * cosf(stackAngle);             // r * cos(u)
        z = moonRadius * sinf(stackAngle);              // r * sin(u)

        for(int j = 0; j <= horizontalSplitCount; ++j){
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi
            
            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle) +2600.0;             // r * cos(u) * sin(v)
            
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            s = (float)j / horizontalSplitCount;
            t = (float)i / verticalSplitCount;
            
            glm::vec3 pos = glm::vec3(x,y,z);
            glm::vec3 norm = glm::normalize(glm::vec3(nx, ny, nz));
            glm::vec2 tex = glm::vec2(s,t);

            moonVertices.push_back(vertex(pos,norm,tex));
        }

    }
    // Initialize Indices of Moon *****************************************************
    int k1, k2 , k1_earth, k2_earth;
    for(int i = 0; i < verticalSplitCount; ++i){
        k1 = i * (horizontalSplitCount + 1);     // beginning of current stack
        k2 = k1 + horizontalSplitCount + 1;      // beginning of next stack

        for(int j = 0; j < horizontalSplitCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if(i != 0)
            {
                moonIndices.push_back(k1);
                moonIndices.push_back(k2);
                moonIndices.push_back(k1 + 1);
            }
            // k1+1 => k2 => k2+1
            if(i != (verticalSplitCount-1))
            {
                moonIndices.push_back(k1 + 1);
                moonIndices.push_back(k2);
                moonIndices.push_back(k2 + 1);
            }
        }
    }

    // FOR EARTH VERTICE ******************************************************************************************
    for(int i = 0 ; i<= verticalSplitCount;++i){
        stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        for(int j = 0; j <= horizontalSplitCount; ++j){
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi
            
            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            s = (float)j / horizontalSplitCount;
            t = (float)i / verticalSplitCount;
            
            glm::vec3 pos = glm::vec3(x,y,z);
            glm::vec3 norm = glm::normalize(glm::vec3(nx, ny, nz));
            glm::vec2 tex = glm::vec2(s,t);

            worldVertices.push_back(vertex(pos,norm,tex));
        }

    }
    // Initialize Indices of Earth *****************************************************
    for(int i = 0; i < verticalSplitCount; ++i){
        k1 = i * (horizontalSplitCount + 1);     // beginning of current stack
        k2 = k1 + horizontalSplitCount + 1;      // beginning of next stack

        for(int j = 0; j < horizontalSplitCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if(i != 0)
            {
                worldIndices.push_back(k1);
                worldIndices.push_back(k2);
                worldIndices.push_back(k1 + 1);
            }
            // k1+1 => k2 => k2+1
            if(i != (verticalSplitCount-1))
            {
                worldIndices.push_back(k1 + 1);
                worldIndices.push_back(k2);
                worldIndices.push_back(k2 + 1);
            }
        }
    }


    glGenVertexArrays(1, &moonVAO);
	glBindVertexArray(moonVAO);

    // init MoonVBO
    glGenBuffers(1, &moonVBO);
    glBindBuffer(GL_ARRAY_BUFFER, moonVBO);
    
    glBufferData(GL_ARRAY_BUFFER, moonVertices.size() * sizeof(vertex), moonVertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &moonEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moonEBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, moonIndices.size() * sizeof(unsigned int), moonIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (sizeof(float)*3));

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (sizeof(float)*6));

    // glBindBuffer(GL_ARRAY_BUFFER,0);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    // glBindVertexArray(0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

    // TODO: Configure Buffers
    

    // World commands
    // Load shaders
    GLuint worldShaderID = initShaders("worldShader.vert", "worldShader.frag");

    initColoredTexture(coloredTexturePath, worldShaderID);
    initGreyTexture(greyTexturePath, worldShaderID);

    // TODO: Set worldVertices
    
    // TODO: Configure Buffers


    glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

    // init worldVBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    //glBufferData(GL_ARRAY_BUFFER, worldVertices.size() * sizeof(vertex), worldVertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, worldIndices.size() * sizeof(unsigned int), worldIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (sizeof(float)*3));

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (sizeof(float)*6));

    // glBindBuffer(GL_ARRAY_BUFFER,0);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    // glBindVertexArray(0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Main rendering loop
    float moonx = 0.0;
    float worldx = 0.0;
    do {

        if(isResized){
            isResized = false;
            screenHeight = resizedHeight;
            screenWidth = resizedWidth;
            aspectRatio = (float) screenWidth/screenHeight;
            M_projection = glm::perspective(projectionAngle, aspectRatio, near, far);
        }
        glViewport(0, 0, screenWidth, screenHeight);
        glClearStencil(0);
        glClearDepth(1.0f);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


        // TODO: Handle key presses
        handleKeyPress(window);
        
        // TODO: Manipulate rotation variables
        
        // TODO: Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, moonTextureColor);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureColor);
        glActiveTexture(GL_TEXTURE2);              
        glBindTexture(GL_TEXTURE_2D, textureGrey);
        
        glBindVertexArray(moonVAO);

        glBindBuffer(GL_ARRAY_BUFFER, moonVBO);
        
        glBufferData(GL_ARRAY_BUFFER, moonVertices.size() * sizeof(vertex), moonVertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, moonEBO);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, moonIndices.size() * sizeof(unsigned int), moonIndices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (sizeof(float)*3));

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (sizeof(float)*6));

        // glBindBuffer(GL_ARRAY_BUFFER,0);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
        // glBindVertexArray(0);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        // TODO: Use moonShaderID program

        glUseProgram(moonShaderID);
        // glBindBuffer(GL_ARRAY_BUFFER,0);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
        // glBindVertexArray(0);
        // TODO: Update camera at every frame
        orbitDegree += 0.02;
        if(orbitDegree >= 360.0){
            orbitDegree = 0.0;
        }
        moonx += (360.0/500.0);
        worldx += (360.0/500.0);
        if(moonx >= 360.0){
            moonx = 0.0;
        }
        if(worldx >= 360.0){
            worldx = 0.0;
        }
        textureOffset -= 0.002;
        // TODO: Update uniform variables at every frame
        cameraPosition += speed * cameraDirection;
        // M_model = glm::translate(M_model, glm::vec3(0.0,+2600.0,0.0));
        // M_model = glm::rotate(M_model, (float) glm::radians(360.0/500.0), glm::vec3(0, 0, 1));
        // M_model = glm::translate(M_model, glm::vec3(0.0,-2600.0,0.0));
        // M_model = glm::rotate(M_model, -(float) glm::radians(1.85), glm::vec3(0, 0, 1));

        M_view = glm::lookAt(cameraPosition, cameraPosition + cameraDirection, cameraUp);
        M_projection = glm::perspective(projectionAngle, aspectRatio, near, far);
        MVP = M_projection * M_view * M_model;
        // inv_Model = glm::transpose(glm::inverse(M_model));
        lightPositionLoc = glGetUniformLocation(moonShaderID, "lightPosition");
        glUniform3fv(lightPositionLoc, 1, glm::value_ptr(lightPos));

        cameraPositionLoc = glGetUniformLocation(moonShaderID, "cameraPosition");
        glUniform3fv(cameraPositionLoc, 1, glm::value_ptr(cameraPosition));

        textureOffsetLoc = glGetUniformLocation(moonShaderID, "textureOffset");
        glUniform1f(textureOffsetLoc, textureOffset);

        heightFactorLoc = glGetUniformLocation(moonShaderID, "heightFactor");
        glUniform1f(heightFactorLoc, heightFactor);

        ProjectionMatrixLoc = glGetUniformLocation(moonShaderID, "ProjectionMatrix");
        glUniformMatrix4fv(ProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(M_projection));

        ViewMatrixLoc = glGetUniformLocation(moonShaderID, "ViewMatrix");
        glUniformMatrix4fv(ViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(M_view));

        MVPLoc = glGetUniformLocation(moonShaderID, "MVP");
        glUniformMatrix4fv(MVPLoc,  1, GL_FALSE, glm::value_ptr(MVP));

        // NormalMatrixLoc = glGetUniformLocation(moonShaderID, "NormalMatrix");
        // glUniformMatrix4fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(inv_Model));

        ModelMatrixLoc = glGetUniformLocation(moonShaderID, "ModelMatrix");
        glUniformMatrix4fv(ModelMatrixLoc, 1, GL_FALSE, glm::value_ptr(M_model));

        TexColorLoc = glGetUniformLocation(moonShaderID, "TexColor");
        glUniform1i(TexColorLoc, 1);

        MoonTexColorLoc = glGetUniformLocation(moonShaderID, "MoonTexColor");
        glUniform1i(MoonTexColorLoc, 0);

        TexGreyLoc = glGetUniformLocation(moonShaderID, "TexGrey");
        glUniform1i(TexGreyLoc,2);


        
        orbitDegreeLoc = glGetUniformLocation(moonShaderID, "orbitDegree");
        glUniform1f(orbitDegreeLoc, orbitDegree);
        
        // TODO: Bind moon vertex array        

        // TODO: Draw moon object
        glDrawElements(GL_TRIANGLES, moonIndices.size(), GL_UNSIGNED_INT, 0);
        /*************************/
        glUseProgram(worldShaderID);

        // TODO: Use worldShaderID program
        // TODO: Update camera at every frame
        // cameraPosition += speed * cameraDirection;
        // M_view = glm::lookAt(cameraPosition, cameraPosition + cameraDirection, cameraUp);
        // M_view = glm::lookAt(cameraPosition, cameraPosition + glm::vec3(cameraDirection.x * near, cameraDirection.y * near, cameraDirection.z * near), cameraUp);
        // M_projection = glm::perspective(projectionAngle, aspectRatio, near, far);
        //M_model3 = glm::rotate(M_model3, (float) glm::radians(300.0/500.0), glm::vec3(0, 0, 1));
        MVP = M_projection * M_view * M_model3;
        // TODO: Update uniform variables at every frame
        lightPositionLoc = glGetUniformLocation(worldShaderID, "lightPosition");
        glUniform3fv(lightPositionLoc, 1, glm::value_ptr(lightPos));

        cameraPositionLoc = glGetUniformLocation(worldShaderID, "cameraPosition");
        glUniform3fv(cameraPositionLoc, 1, glm::value_ptr(cameraPosition));

        textureOffsetLoc = glGetUniformLocation(worldShaderID, "textureOffset");
        glUniform1f(textureOffsetLoc, textureOffset);

        heightFactorLoc = glGetUniformLocation(worldShaderID, "heightFactor");
        glUniform1f(heightFactorLoc, heightFactor);

        ProjectionMatrixLoc = glGetUniformLocation(worldShaderID, "ProjectionMatrix");
        glUniformMatrix4fv(ProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(M_projection));

        ModelMatrixLoc = glGetUniformLocation(worldShaderID, "ModelMatrix");
        glUniformMatrix4fv(ModelMatrixLoc, 1, GL_FALSE, glm::value_ptr(M_model3));

        ViewMatrixLoc = glGetUniformLocation(worldShaderID, "ViewMatrix");
        glUniformMatrix4fv(ViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(M_view));

        MVPLoc = glGetUniformLocation(worldShaderID, "MVP");
        glUniformMatrix4fv(MVPLoc,  1, GL_FALSE, glm::value_ptr(MVP));

        TexColorLoc = glGetUniformLocation(worldShaderID, "TexColor");
        glUniform1i(TexColorLoc, 1);

        MoonTexColorLoc = glGetUniformLocation(worldShaderID, "MoonTexColor");
        glUniform1i(MoonTexColorLoc, 0);

        TexGreyLoc = glGetUniformLocation(worldShaderID, "TexGrey");
        glUniform1i(TexGreyLoc,2);

        // TODO: Bind world vertex array

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        
        glBufferData(GL_ARRAY_BUFFER, worldVertices.size() * sizeof(vertex), worldVertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, worldIndices.size() * sizeof(unsigned int), worldIndices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (sizeof(float)*3));

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (sizeof(float)*6));

        // glBindBuffer(GL_ARRAY_BUFFER,0);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
        // glBindVertexArray(0);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        // TODO: Draw world object
        glDrawElements(GL_TRIANGLES, worldIndices.size(), GL_UNSIGNED_INT, 0);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window));

    // Delete buffers
    glDeleteBuffers(1, &moonVAO);
    glDeleteBuffers(1, &moonVBO);
    glDeleteBuffers(1, &moonEBO);

    
    // Delete buffers
    glDeleteBuffers(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
   
    glDeleteProgram(moonShaderID);
    glDeleteProgram(worldShaderID);

    // Close window
    glfwTerminate();
}

void EclipseMap::handleKeyPress(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    // spec #14
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS){
        EclipseMap::cameraPosition = EclipseMap::cameraStartPosition;
        EclipseMap::cameraDirection = EclipseMap::cameraStartDirection;
        EclipseMap::cameraUp = EclipseMap::cameraStartUp;
        EclipseMap::speed = EclipseMap::startSpeed;
    }

    // spec #15
    if ((glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) && glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE){
        if(displayFormat == displayFormatOptions::fullScreen){
            cout << "convert display format to windowed\n";
            screenWidth = defaultScreenWidth;
            screenHeight = defaultScreenHeight;
            displayFormat = displayFormatOptions::windowed;
            glfwSetWindowMonitor(window, nullptr, 0, 0, screenWidth, screenHeight, 0);
            glViewport(0, 0, screenWidth, screenHeight);
            aspectRatio = 1;
        }
        else {
            displayFormat = displayFormatOptions::fullScreen;

            glfwGetWindowPos(window, &screenWidth, &screenHeight);
            glfwGetWindowSize(window, &screenWidth, &screenHeight);

            auto monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);

            screenWidth = mode->width;
            screenHeight = mode->height;
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            glViewport(0, 0, mode->width, mode->height);

            aspectRatio = (float) mode->width/mode->height;
        }
    }

    // spec #13
    if(glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS){
        speed += 0.01;
    }
    if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS){
        speed -= 0.01;
    }
    if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
        speed = 0;
    }
    //increase pitch (rotate around left vector)
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        glm::vec3 cameraLeft = glm::cross(cameraUp, cameraDirection);
        cameraDirection = glm::rotate(cameraDirection, -0.05f, cameraLeft);
        cameraUp = glm::rotate(cameraUp, -0.05f, cameraLeft);
    }
    //decrease pitch
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        glm::vec3 cameraLeft = glm::cross(cameraUp, cameraDirection);
        cameraDirection = glm::rotate(cameraDirection, 0.05f, cameraLeft);
        cameraUp = glm::rotate(cameraUp, 0.05f, cameraLeft);
    }
    //increase pitch (rotate around up vector)
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        cameraDirection = glm::rotate(cameraDirection, -0.05f, cameraUp);
    }
    //decrease pitch
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        cameraDirection = glm::rotate(cameraDirection, 0.05f, cameraUp);
    }

    //spec #5
    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
        heightFactor += 10;
    }
    if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS){
        heightFactor -= 10;
        if(heightFactor < 0){
            heightFactor = 0;
        }
    }


}

GLFWwindow *EclipseMap::openWindow(const char *windowName, int width, int height) {
    if (!glfwInit()) {
        getchar();
        return 0;
    }

    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(width, height, windowName, NULL, NULL);
    glfwSetWindowMonitor(window, NULL, 1, 31, screenWidth, screenHeight, mode->refreshRate);

    if (window == NULL) {
        getchar();
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        getchar();
        glfwTerminate();
        return 0;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glClearColor(0, 0, 0, 0);

    return window;
}


void EclipseMap::initColoredTexture(const char *filename, GLuint shader) {
    int width, height;
    glGenTextures(1, &textureColor);
    cout << shader << endl;
    glBindTexture(GL_TEXTURE_2D, textureColor);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char *raw_image = NULL;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1];

    FILE *infile = fopen(filename, "rb");
    unsigned long location = 0;
    int i = 0, j = 0;

    if (!infile) {
        printf("Error opening jpeg file %s\n!", filename);
        return;
    }
    printf("Texture filename = %s\n", filename);

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error(&jerr);
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress(&cinfo);
    /* this makes the library read from infile */
    jpeg_stdio_src(&cinfo, infile);
    /* reading the image header which contains image information */
    jpeg_read_header(&cinfo, TRUE);
    /* Start decompression jpeg here */
    jpeg_start_decompress(&cinfo);

    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char *) malloc(cinfo.output_width * cinfo.output_height * cinfo.num_components);
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *) malloc(cinfo.output_width * cinfo.num_components);
    /* read one scan line at a time */
    while (cinfo.output_scanline < cinfo.image_height) {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
        for (i = 0; i < cinfo.image_width * cinfo.num_components; i++)
            raw_image[location++] = row_pointer[0][i];
    }

    height = cinfo.image_height;
    width = cinfo.image_width;


    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);
   

    imageWidth = width;
    imageHeight = height;

    glGenerateMipmap(GL_TEXTURE_2D);

    //glUseProgram(shader); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:

    //glUniform1i(glGetUniformLocation(shader, "TexColor"), 0);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
    free(raw_image);
    fclose(infile);

}

void EclipseMap::initGreyTexture(const char *filename, GLuint shader) {

    glGenTextures(1, &textureGrey);
    glBindTexture(GL_TEXTURE_2D, textureGrey);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height;

    unsigned char *raw_image = NULL;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1];

    FILE *infile = fopen(filename, "rb");
    unsigned long location = 0;
    int i = 0, j = 0;

    if (!infile) {
        printf("Error opening jpeg file %s\n!", filename);
        return;
    }
    printf("Texture filename = %s\n", filename);

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error(&jerr);
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress(&cinfo);
    /* this makes the library read from infile */
    jpeg_stdio_src(&cinfo, infile);
    /* reading the image header which contains image information */
    jpeg_read_header(&cinfo, TRUE);
    /* Start decompression jpeg here */
    jpeg_start_decompress(&cinfo);

    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char *) malloc(cinfo.output_width * cinfo.output_height * cinfo.num_components);
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *) malloc(cinfo.output_width * cinfo.num_components);
    /* read one scan line at a time */
    while (cinfo.output_scanline < cinfo.image_height) {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
        for (i = 0; i < cinfo.image_width * cinfo.num_components; i++)
            raw_image[location++] = row_pointer[0][i];
    }

    height = cinfo.image_height;
    width = cinfo.image_width;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);
  



    glGenerateMipmap(GL_TEXTURE_2D);

    //glUseProgram(shader); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:

    //glUniform1i(glGetUniformLocation(shader, "TexGrey"), 1);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
    free(raw_image);
    fclose(infile);

}

void EclipseMap::initMoonColoredTexture(const char *filename, GLuint shader) {
    int width, height;
    glGenTextures(1, &moonTextureColor);
    cout << shader << endl;
    glBindTexture(GL_TEXTURE_2D, moonTextureColor);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char *raw_image = NULL;
    int bytes_per_pixel = 3;   /* or 1 for GRACYSCALE images */
    int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */

    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1];

    FILE *infile = fopen(filename, "rb");
    unsigned long location = 0;
    int i = 0, j = 0;

    if (!infile) {
        printf("Error opening jpeg file %s\n!", filename);
        return;
    }
    printf("Texture filename = %s\n", filename);

    /* here we set up the standard libjpeg error handler */
    cinfo.err = jpeg_std_error(&jerr);
    /* setup decompression process and source, then read JPEG header */
    jpeg_create_decompress(&cinfo);
    /* this makes the library read from infile */
    jpeg_stdio_src(&cinfo, infile);
    /* reading the image header which contains image information */
    jpeg_read_header(&cinfo, TRUE);
    /* Start decompression jpeg here */
    jpeg_start_decompress(&cinfo);

    /* allocate memory to hold the uncompressed image */
    raw_image = (unsigned char *) malloc(cinfo.output_width * cinfo.output_height * cinfo.num_components);
    /* now actually read the jpeg into the raw buffer */
    row_pointer[0] = (unsigned char *) malloc(cinfo.output_width * cinfo.num_components);
    /* read one scan line at a time */
    while (cinfo.output_scanline < cinfo.image_height) {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
        for (i = 0; i < cinfo.image_width * cinfo.num_components; i++)
            raw_image[location++] = row_pointer[0][i];
    }

    height = cinfo.image_height;
    width = cinfo.image_width;


    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_image);
   

    imageWidth = width;
    imageHeight = height;

    glGenerateMipmap(GL_TEXTURE_2D);

    //glUseProgram(shader); // don't forget to activate/use the shader before setting uniforms!
    // either set it manually like so:

    //glUniform1i(glGetUniformLocation(shader, "MoonTexColor"), 2);
    /* wrap up decompression, destroy objects, free pointers and close open files */
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row_pointer[0]);
    free(raw_image);
    fclose(infile);

}
