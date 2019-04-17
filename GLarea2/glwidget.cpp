/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "glwidget.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMouseEvent>

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      clearColor(Qt::GlobalColor::white),
      xRot(0),
      yRot(0),
      zRot(0),
      program(0)
{
	SCR_WIDTH = this->frameGeometry().width();
	SCR_HEIGHT = this->frameGeometry().height();

	orth::ModelRead mr("./0016.ply", mm);
	TeethSegmentRun("./0016.txt");
}

GLWidget::~GLWidget()
{
    makeCurrent();
    vbo.destroy();
	vboPlane.destroy();
	vao.destroy();
	vaoPlane.destroy();
    delete program;
    doneCurrent();
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(500, 500);
}

QSize GLWidget::sizeHint() const
{
    return QSize(1000, 1000);
}

void GLWidget::setWindowSize(QSize &size)
{
	SCR_WIDTH = size.width();
	SCR_HEIGHT = size.height();
	/*cout << "SCR_WIDTH = " << SCR_WIDTH << endl;
	cout << "SCR_HEIGHT = " << SCR_HEIGHT << endl;*/
}

void GLWidget::rotateBy(int xAngle, int yAngle, int zAngle)
{
    xRot += xAngle;
    yRot += yAngle;
    zRot += zAngle;
    update();
}

void GLWidget::setClearColor(const QColor &color)
{
    clearColor = color;
    update();
}

//const char* GLWidget::ReadShader(const char* Path)
//{
//	std::cout << Path << std::endl;
//	// 1. retrieve the vertex/fragment source code from filePath
//	std::string vertexCode;
//	std::ifstream vShaderFile;
//	// ensure ifstream objects can throw exceptions:
//	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
//	try
//	{
//		// open files
//		vShaderFile.open(Path);
//		std::stringstream vShaderStream, fShaderStream;
//		// read file's buffer contents into streams
//		vShaderStream << vShaderFile.rdbuf();
//		// close file handlers
//		vShaderFile.close();
//		// convert stream into string
//		vertexCode = vShaderStream.str();
//	}
//	catch (std::ifstream::failure e)
//	{
//		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
//	}
//	const char* vShaderCode = vertexCode.c_str();
//	std::cout << vertexCode << std::endl;
//	return vShaderCode;
//	
//}

QByteArray GLWidget::ReadShader(const QString &Path)
{
	QByteArray data;
	QFile qrcFile(Path);
	if (qrcFile.open(QIODevice::ReadOnly))
	{
		data = qrcFile.readAll();
	}
	else {
		qWarning() << "Failed to load input Shader file, falling back to default";
	}
	return data;

}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
	//createGeometry();
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_NORMAL_ATTRIBUTE 1
//#define PROGRAM_MATERIAL_ATTRIBUTE 2
	{
		//����shader
		QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
		vshader->compileSourceCode(ReadShader(":/gl2.vs"));
		QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
		fshader->compileSourceCode(ReadShader(":/gl2.fs"));

		program = new QOpenGLShaderProgram;
		program->addShader(vshader);
		program->addShader(fshader);
		program->bindAttributeLocation("aPos", PROGRAM_VERTEX_ATTRIBUTE);
		program->bindAttributeLocation("aNormal", PROGRAM_NORMAL_ATTRIBUTE);
		makeObject();
	}
	{
		//ƽ��shader
		QOpenGLShader *vs = new QOpenGLShader(QOpenGLShader::Vertex, this);
		vs->compileSourceCode(ReadShader(":/plane.vs"));
		QOpenGLShader *fs = new QOpenGLShader(QOpenGLShader::Fragment, this);
		fs->compileSourceCode(ReadShader(":/plane.fs"));

		programPlane = new QOpenGLShaderProgram;
		programPlane->addShader(vs);
		programPlane->addShader(fs);
		programPlane->bindAttributeLocation("aPos", PROGRAM_VERTEX_ATTRIBUTE);
		programPlane->bindAttributeLocation("aNormal", PROGRAM_NORMAL_ATTRIBUTE);

		float planeVertices[] = {
			// positions            // normals         
			 50.0, -4.0f,  50.0, 0.0f, 1.0f, 0.0f,
			-50.0, -4.0f,  50.0, 0.0f, 1.0f, 0.0f,
			-50.0, -4.0f, -50.0, 0.0f, 1.0f, 0.0f,

			 50.0, -4.0f,  50.0, 0.0f, 1.0f, 0.0f,
			-50.0, -4.0f, -50.0, 0.0f, 1.0f, 0.0f,
			 50.0, -4.0f, -50.0, 0.0f, 1.0f, 0.0f,
		};
		vboPlane.create();
		vboPlane.bind();
		vboPlane.setUsagePattern(QOpenGLBuffer::StaticDraw);
		vboPlane.allocate(planeVertices, sizeof(planeVertices));

		vaoPlane.create();
		vaoPlane.bind();
		programPlane->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
		programPlane->enableAttributeArray(PROGRAM_NORMAL_ATTRIBUTE);
		programPlane->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
		programPlane->setAttributeBuffer(PROGRAM_NORMAL_ATTRIBUTE, GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

		programPlane->link();
		programPlane->bind();
		vaoPlane.release();
	}
	{
		QOpenGLShader *vsLight = new QOpenGLShader(QOpenGLShader::Vertex, this);
		vsLight->compileSourceCode(ReadShader(":/lightSpace.vs"));
		QOpenGLShader *fsLight = new QOpenGLShader(QOpenGLShader::Fragment, this);
		fsLight->compileSourceCode(ReadShader(":/lightSpace.fs"));
		programLightSpace = new QOpenGLShaderProgram;
		programLightSpace->addShader(vsLight);
		programLightSpace->addShader(fsLight);
		programLightSpace->link();
		programLightSpace->bind();
	}
	setClearColor(QColor(187, 255, 255));
	//setClearColor(QColor(255, 255, 255));
	glGenFramebuffers(1, &depthMapFBO);
	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLWidget::paintGL()
{
	
    glClearColor(clearColor.redF(), clearColor.greenF(), clearColor.blueF(), clearColor.alphaF());
	//cout << clearColor.redF() << " ;" << clearColor.greenF() << " ;" << clearColor.blueF() << " ;" << endl;
	//glClearColor()
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//���ͶӰ�ռ�
	QVector3D lightPos(QVector3D(0, 50, 10));
	QMatrix4x4 lightProjection, lightView, lightSpaceMatrix;
	QMatrix4x4 modelLight;
	modelLight.setToIdentity();
	/*modelLight.rotate(xRot / 16.0f, 1.0f, 0.0f, 0.0f);
	modelLight.rotate(yRot / 16.0f, 0.0f, 1.0f, 0.0f);
	modelLight.rotate(zRot / 16.0f, 0.0f, 0.0f, 1.0f);*/
	float near_plane = 1.0f, far_plane = 200.0f;
	lightProjection.ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);
	lightView.lookAt(lightPos, QVector3D(0.0f, -10.0f, 0.0f), QVector3D(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView * modelLight;
	programLightSpace->bind();
	programLightSpace->setUniformValue("lightSpaceMatrix", lightSpaceMatrix);
	//programLightSpace->setUniformValue("model", modelLight);
	//glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	//�򵥻������壬�õ��������
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	programLightSpace->bindAttributeLocation("aPos", 0);
	programLightSpace->enableAttributeArray(0);
	programLightSpace->setAttributeBuffer(0, GL_FLOAT, 0, 3, 7 * sizeof(GLfloat));
	vao.bind();
	glDrawArrays(GL_TRIANGLES, 0, vertices_in.size() / 3);
	vao.release();
	/*programLightSpace->enableAttributeArray(0);
	programLightSpace->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
	vaoPlane.bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	vaoPlane.release();*/
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//������Ⱦ
	program->bind();
    //QMatrix4x4 m;
    //m.ortho(-5.0f, +5.0f, +5.0f, -5.0f, -5.0f, 35.0f);
    //m.translate(0.0f, 0.0f, -10.0f);
    //m.rotate(xRot / 16.0f, 1.0f, 0.0f, 0.0f);
    //m.rotate(yRot / 16.0f, 0.0f, 1.0f, 0.0f);
    //m.rotate(zRot / 16.0f, 0.0f, 0.0f, 1.0f);

	QMatrix4x4 projection, view, model;
	projection.setToIdentity();
	projection.perspective(FOV, (float)SCR_WIDTH / (float)SCR_HEIGHT, 150.0f, 300.0f);
	program->setUniformValue("projection", projection);

	//cout << projection.data()[0] << ", " << projection.data()[4] << ", " << projection.data()[8] << ", " << projection.data()[12] <<  endl;
	//cout << projection.data()[1] << ", " << projection.data()[5] << ", " << projection.data()[9] << ", " << projection.data()[13] << endl;
	//cout << projection.data()[2] << ", " << projection.data()[6] << ", " << projection.data()[10] << ", " << projection.data()[14] << endl;
	//cout << projection.data()[3] << ", " << projection.data()[7] << ", " << projection.data()[11] << ", " << projection.data()[15] << endl;
	//cout << endl;

	view.setToIdentity();
	view.translate(0.0f, -10.0f, -230.0f);
	view.rotate(xRot / 16.0f, 1.0f, 0.0f, 0.0f);
	view.rotate(yRot / 16.0f, 0.0f, 1.0f, 0.0f);
	view.rotate(zRot / 16.0f, 0.0f, 0.0f, 1.0f);
	//view.rotate(xRot / 16.0f, 1.0f, 0.0f, 0.0f);
	//view.rotate(yRot / 16.0f, 0.0f, 1.0f, 0.0f);
	//view.rotate(zRot / 16.0f, 0.0f, 0.0f, 1.0f);
	program->setUniformValue("view", view);
	program->setUniformValue("viewPos", view.column(3));
	//cout << view.data()[0] << ", " << view.data()[4] << ", " << view.data()[8] << ", " << view.data()[12] << endl;
	//cout << view.data()[1] << ", " << view.data()[5] << ", " << view.data()[9] << ", " << view.data()[13] << endl;
	//cout << view.data()[2] << ", " << view.data()[6] << ", " << view.data()[10] << ", " << view.data()[14] << endl;
	//cout << view.data()[3] << ", " << view.data()[7] << ", " << view.data()[11] << ", " << view.data()[15] << endl;
	//cout << endl;

	model.setToIdentity();
	//cout << model.data()[0] << ", " << model.data()[4] << ", " << model.data()[8] << ", " << model.data()[12] << endl;
	//cout << model.data()[1] << ", " << model.data()[5] << ", " << model.data()[9] << ", " << model.data()[13] << endl;
	//cout << model.data()[2] << ", " << model.data()[6] << ", " << model.data()[10] << ", " << model.data()[14] << endl;
	//cout << model.data()[3] << ", " << model.data()[7] << ", " << model.data()[11] << ", " << model.data()[15] << endl;
	//cout << endl;
	//vector<float> a(4,0);
	//cout << a[0] << ", " << a[1] << ", " << a[2] << ", " << a[3] << endl;
	//for (int i = 0; i < 4; i++)
	//{
	//	for (int j = 0; j < 4; j++)
	//	{
	//		a[j] += model.data()[j + 4 * i] * fv[i];
	//	}
	//}
	//cout << a[0] << ", " << a[1] << ", " << a[2] << ", " << a[3] << endl;
	//cout << "-----------------------------" << endl;
	//vector<float> a1(4, 0);
	//for (int i = 0; i < 4; i++)
	//{
	//	a1[0] += view.column(i).x()*a[i];
	//	a1[1] += view.column(i).y()*a[i];
	//	a1[2] += view.column(i).z()*a[i];
	//	a1[3] += view.column(i).w()*a[i];
	//	
	//}
	//cout << a1[0] << ", " << a1[1] << ", " << a1[2] << ", " << a1[3] << endl;cout << "-----------------------------" << endl;
	//vector<float> a2(4, 0);
	//for (int i = 0; i < 4; i++)
	//{
	//	a2[0] += projection.column(i).x() *a1[i];
	//	a2[1] += projection.column(i).y() *a1[i];
	//	a2[2] += projection.column(i).z() *a1[i];
	//	a2[3] += projection.column(i).w() *a1[i];
	//	
	//}
	//cout << a2[0] << ", " << a2[1] << ", " << a2[2] << ", " << a2[3] << endl;cout << "-----------------------------" << endl;//cout << model.data()[0] << ", " << model.data()[4] << ", " << model.data()[8] << ", " << model.data()[12] << endl;
	//cout << model.data()[1] << ", " << model.data()[5] << ", " << model.data()[9] << ", " << model.data()[13] << endl;
	//cout << model.data()[2] << ", " << model.data()[6] << ", " << model.data()[10] << ", " << model.data()[14] << endl;
	//cout << model.data()[3] << ", " << model.data()[7] << ", " << model.data()[11] << ", " << model.data()[15] << endl;
	//cout << endl;
	//vector<float> a(4,0);
	//cout << a[0] << ", " << a[1] << ", " << a[2] << ", " << a[3] << endl;
	//for (int i = 0; i < 4; i++)
	//{
	//	for (int j = 0; j < 4; j++)
	//	{
	//		a[j] += model.data()[j + 4 * i] * fv[i];
	//	}
	//}
	//cout << a[0] << ", " << a[1] << ", " << a[2] << ", " << a[3] << endl;
	//cout << "-----------------------------" << endl;
	//vector<float> a1(4, 0);
	//for (int i = 0; i < 4; i++)
	//{
	//	a1[0] += view.column(i).x()*a[i];
	//	a1[1] += view.column(i).y()*a[i];
	//	a1[2] += view.column(i).z()*a[i];
	//	a1[3] += view.column(i).w()*a[i];
	//	
	//}
	//cout << a1[0] << ", " << a1[1] << ", " << a1[2] << ", " << a1[3] << endl;cout << "-----------------------------" << endl;
	//vector<float> a2(4, 0);
	//for (int i = 0; i < 4; i++)
	//{
	//	a2[0] += projection.column(i).x() *a1[i];
	//	a2[1] += projection.column(i).y() *a1[i];
	//	a2[2] += projection.column(i).z() *a1[i];
	//	a2[3] += projection.column(i).w() *a1[i];
	//	
	//}
	//cout << a2[0] << ", " << a2[1] << ", " << a2[2] << ", " << a2[3] << endl;cout << "-----------------------------" << endl;
	//model.translate(xRot, yRot, zRot);
	program->setUniformValue("model", model);
	program->setUniformValue("lightPos", lightPos);
	//cout << view.column(3).toVector3D()[0] <<", " << view.column(3).toVector3D()[1] << ", " << view.column(3).toVector3D()[2]<< endl;
	program->setUniformValue("lightColor", QVector3D(0.7f,0.7f, 0.7f));
	program->setUniformValue("objectColor", QVector3D(1.2f, 0.7f, 0.71f));
	//program->setUniformValue("gingivaColor", QVector3D(1.3f, 1.3f, 1.3f));
	//program->setUniformValue("teethColor", QVector3D(1.3f, 1.3f, 1.3f));
	
	program->setUniformValue("material1.ambient", 0.6f, 0.0f, 0.0f);
	program->setUniformValue("material1.diffuse", 1.0f, 0.756863f, 0.756863f);
	program->setUniformValue("material1.specular", 0.8f,0.04f,0.04f);
	program->setUniformValue("material1.shininess", 0.978125f);//0.978125f

	program->setUniformValue("material2.ambient", 1.2f,1.2f,1.2f);
	program->setUniformValue("material2.diffuse", 1.0f,	1.0f,1.0f);
	program->setUniformValue("material2.specular", 1.2f, 1.2f, 1.2f);
	program->setUniformValue("material2.shininess", 32.0f);

	// directional light
	program->setUniformValue("dirLight.direction", 0.0f, 0.0f, 1.0f);
	program->setUniformValue("dirLight.ambient", 0.45f, 0.45f, 0.45f);
	program->setUniformValue("dirLight.diffuse", 0.6f, 0.6f, 0.6f);
	program->setUniformValue("dirLight.specular", 0.7f, 0.7f, 0.7f);
	//// point light 1
	//program->setUniformValue("pointLights[0].position", 0.7f, 0.2f, 2.0f);
	//program->setUniformValue("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	//program->setUniformValue("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	//program->setUniformValue("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	//program->setUniformValue("pointLights[0].constant", 1.0f);
	//program->setUniformValue("pointLights[0].linear", 0.09f);
	//program->setUniformValue("pointLights[0].quadratic", 0.032f);
	//// point light 2
	//program->setUniformValue("pointLights[1].position", 2.3f, -3.3f, -4.0f);
	//program->setUniformValue("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	//program->setUniformValue("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
	//program->setUniformValue("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
	//program->setUniformValue("pointLights[1].constant", 1.0f);
	//program->setUniformValue("pointLights[1].linear", 0.09f);
	//program->setUniformValue("pointLights[1].quadratic", 0.032f);
	//// point light 3
	//program->setUniformValue("pointLights[2].position", -4.0f, 2.0f, -12.0f);
	//program->setUniformValue("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	//program->setUniformValue("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
	//program->setUniformValue("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
	//program->setUniformValue("pointLights[2].constant", 1.0f);
	//program->setUniformValue("pointLights[2].linear", 0.09f);
	//program->setUniformValue("pointLights[2].quadratic", 0.032f);
	//// point light 4
	//program->setUniformValue("pointLights[3].position", 0.0f, 0.0f, -3.0f);
	//program->setUniformValue("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
	//program->setUniformValue("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
	//program->setUniformValue("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
	//program->setUniformValue("pointLights[3].constant", 1.0f);
	//program->setUniformValue("pointLights[3].linear", 0.09f);
	//program->setUniformValue("pointLights[3].quadratic", 0.032f);
	//// spotLight
	//program->setUniformValue("spotLight.position", 0.0f, 5.0f, 20.0f);
	//program->setUniformValue("spotLight.direction", 0.0f, -1.0f, .0f);
	//program->setUniformValue("spotLight.ambient", 1.0f, 1.0f, 1.0f);
	//program->setUniformValue("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	//program->setUniformValue("spotLight.specular", 1.0f, 1.0f, 1.0f);
	//program->setUniformValue("spotLight.constant", 1.0f);;
	//program->setUniformValue("spotLight.linear", 0.045f);
	//program->setUniformValue("spotLight.quadratic", 0.0075f);
	//program->setUniformValue("spotLight.cutOff", float(cos(17.5)));
	//program->setUniformValue("spotLight.outerCutOff", float(cos(20)));
 //   program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
 //   program->enableAttributeArray(PROGRAM_NORMAL_ATTRIBUTE);
	////program->enableAttributeArray(PROGRAM_MATERIAL_ATTRIBUTE);
 //   program->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 4, 7 * sizeof(GLfloat));
 //   program->setAttributeBuffer(PROGRAM_NORMAL_ATTRIBUTE, GL_FLOAT, 4 * sizeof(GLfloat), 3, 7 * sizeof(GLfloat));
	////program->setAttributeBuffer(PROGRAM_MATERIAL_ATTRIBUTE, GL_FLOAT, 0, 1, 7 * sizeof(GLfloat));

	//��������������������ƽ����
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	vao.bind();
	glDrawArrays(GL_TRIANGLES, 0, vertices_in.size()/3);
	vao.release();
	//���㵱ǰ�ӵ㷽����ƽ�淨�����ĵ���������Ϊ��ʱ������ƽ��
	QVector3D planeNormal(0.0f, 1.0f, 0.0f);
	planeNormal = (view * model.inverted().transposed() * planeNormal).normalized();
	//std::cout << planeNormal.x() << "," << planeNormal.y() << "," << planeNormal.z() << std::endl;
	QVector3D viewPos(view.column(3).x(), view.column(3).y(), view.column(3).z());
	/*std::cout << view.column(3).x() << "," << view.column(3).y() << "," << view.column(3).z() << std::endl;
	std::cout << view.column(3).x() << "," << view.column(3).y() << "," << view.column(3).z() << "," << view.column(3).w() << std::endl;*/
	QVector3D planeCenter(0.0f, -4.0f, 0.0f);
	planeCenter = view * model * planeCenter;
	//std::cout << planeCenter.x() << "," << planeCenter.y() << "," << planeCenter.z() << std::endl;
	QVector3D viewDir;
	viewDir = (planeCenter - viewPos).normalized();
	//std::cout << viewDir.x() << "," << viewDir.y() << "," << viewDir.z() << std::endl;
	float dotValue = QVector3D::dotProduct(planeNormal, viewDir);
	//std::cout << dotValue << std::endl;
	if (dotValue >= 0) {
		programPlane->bind();
		programPlane->setUniformValue("projection", projection);
		programPlane->setUniformValue("view", view);
		programPlane->setUniformValue("model", model);
		programPlane->setUniformValue("lightPos", lightPos);
		programPlane->setUniformValue("viewPos", view.column(3));
		//programPlane->setUniformValue("inv_model", model.inverted());
		programPlane->setUniformValue("color", QVector3D(clearColor.redF(), clearColor.greenF(), clearColor.blueF()));
		programPlane->setUniformValue("lightSpaceMatrix", lightSpaceMatrix);
		programPlane->setUniformValue("shadowMap", 0);
		//programPlane->setUniformValue("color", QVector3D(0.733f, 1, 1));
		programPlane->setUniformValue("alpha", 0.4f);

		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		vaoPlane.bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		//glDisable(GL_BLEND);
		vaoPlane.release();
	}
}

void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();
	//cout << dx << " ----- " << dy << endl;

	//if (event->buttons()&Qt::LeftButton)
	//{
	//	if (event->modifiers() == Qt::CTRL)
	//	{
	//		program->executeTranslateOperation(event->x(), event->y());
	//	}
	//	else
	//	{
	//		program->executeRotateOperation(event->x(), event->y());
	//	}
	//}

    if (event->buttons() & Qt::LeftButton) {
		rotateBy(8 * dy, 0, 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        rotateBy(8 * dy, 8 * dx, 0);
    }
    lastPos = event->pos();
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
	if (event->delta()>0)
	{
		FOV += 1.0f;
	}
	else
	{
		FOV -= 1.0f;
	}
	this->repaint();
}

void GLWidget::mouseReleaseEvent(QMouseEvent * /* event */)
{
    emit clicked();
}

void GLWidget::makeObject()
{
	//fv[0] = mm.P[mm.F[0].x].x / 1.0;
	//fv[1] = mm.P[mm.F[0].x].y / 1.0;
	//fv[2] = mm.P[mm.F[0].x].z / 1.0;
	//fv[3] = 1;

	for (int i = 0; i < mm.F.size(); i++)
	{
		vertices_in.push_back(mm.P[mm.F[i].x].x);
		vertices_in.push_back(mm.P[mm.F[i].x].y);
		vertices_in.push_back(mm.P[mm.F[i].x].z);
		label_in.push_back((float)mm.L[mm.F[i].x]);

		vertices_in.push_back(mm.P[mm.F[i].y].x);
		vertices_in.push_back(mm.P[mm.F[i].y].y);
		vertices_in.push_back(mm.P[mm.F[i].y].z);
		label_in.push_back((float)mm.L[mm.F[i].y]);

		vertices_in.push_back(mm.P[mm.F[i].z].x);
		vertices_in.push_back(mm.P[mm.F[i].z].y);
		vertices_in.push_back(mm.P[mm.F[i].z].z);
		label_in.push_back((float)mm.L[mm.F[i].z]);


	}

	if (mm.N.size()!=0)
	{
		
		for (int i = 0; i < mm.F.size(); i++)
		{
			normal_in.push_back(mm.N[mm.F[i].x].x);
			normal_in.push_back(mm.N[mm.F[i].x].y);
			normal_in.push_back(mm.N[mm.F[i].x].z);

			normal_in.push_back(mm.N[mm.F[i].y].x);
			normal_in.push_back(mm.N[mm.F[i].y].y);
			normal_in.push_back(mm.N[mm.F[i].y].z);

			normal_in.push_back(mm.N[mm.F[i].z].x);
			normal_in.push_back(mm.N[mm.F[i].z].y);
			normal_in.push_back(mm.N[mm.F[i].z].z);
		}
	}

	float min_y = 1, max_y = -1;
	for (int i = 0; i < vertices_in.size()/3; i++)
	{
		float y = vertices_in[3 * i + 2] / 1.0;
		if (min_y>y)
		{
			min_y = y;
		}
		if (max_y<y)
		{
			max_y = y;
		}
	}

    QVector<GLfloat> vertData;
	if (vertices_in.size()>0)
	{
		
		for (int i = 0; i < vertices_in.size()/3; i++)
		{
			float x, y, z;
			x = vertices_in[3 * i + 0] / 1.0;
			y = vertices_in[3 * i + 1] / 1.0;
			z = vertices_in[3 * i + 2] / 1.0;
			vertData.append(x);
			vertData.append(y);
			vertData.append(z);
			vertData.append(label_in[i]);
			vertData.append(normal_in[3 * i + 0]);
			vertData.append(normal_in[3 * i + 1]);
			vertData.append(normal_in[3 * i + 2]);
			
		}
	}
	//if (m_vertices.size()>0)
	//{
	//	for (int i = 0; i < m_vertices.size(); i++)
	//	{
	//		vertData.append(m_vertices[i].x());
	//		vertData.append(m_vertices[i].y());
	//		vertData.append(m_vertices[i].z());
	//		vertData.append(m_normals[i].x());
	//		vertData.append(m_normals[i].y());
	//		vertData.append(m_normals[i].z());
	//	}	
	//}

    vbo.create();
    vbo.bind();
	vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));

	vao.create();
	vao.bind();
	program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
	program->enableAttributeArray(PROGRAM_NORMAL_ATTRIBUTE);
	program->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 4, 7 * sizeof(GLfloat));
	program->setAttributeBuffer(PROGRAM_NORMAL_ATTRIBUTE, GL_FLOAT, 4 * sizeof(GLfloat), 3, 7 * sizeof(GLfloat));

	program->link();
	program->bind();
	vao.release();
}

void GLWidget::createGeometry()
{
	m_vertices.clear();
	m_normals.clear();

	qreal x1 = +0.06f;
	qreal y1 = -0.14f;
	qreal x2 = +0.14f;
	qreal y2 = -0.06f;
	qreal x3 = +0.08f;
	qreal y3 = +0.00f;
	qreal x4 = +0.30f;
	qreal y4 = +0.22f;

	quad(x1, y1, x2, y2, y2, x2, y1, x1);
	quad(x3, y3, x4, y4, y4, x4, y3, x3);

	extrude(x1, y1, x2, y2);
	extrude(x2, y2, y2, x2);
	extrude(y2, x2, y1, x1);
	extrude(y1, x1, x1, y1);
	extrude(x3, y3, x4, y4);
	extrude(x4, y4, y4, x4);
	extrude(y4, x4, y3, x3);

	const qreal Pi = 3.14159f;
	const int NumSectors = 100;

	for (int i = 0; i < NumSectors; ++i) {
		qreal angle1 = (i * 2 * Pi) / NumSectors;
		qreal x5 = 0.30 * sin(angle1);
		qreal y5 = 0.30 * cos(angle1);
		qreal x6 = 0.20 * sin(angle1);
		qreal y6 = 0.20 * cos(angle1);

		qreal angle2 = ((i + 1) * 2 * Pi) / NumSectors;
		qreal x7 = 0.20 * sin(angle2);
		qreal y7 = 0.20 * cos(angle2);
		qreal x8 = 0.30 * sin(angle2);
		qreal y8 = 0.30 * cos(angle2);

		quad(x5, y5, x6, y6, x7, y7, x8, y8);

		extrude(x6, y6, x7, y7);
		extrude(x8, y8, x5, y5);
	}

	for (int i = 0; i < m_vertices.size(); i++)
		m_vertices[i] *= 0.5f;
}

void GLWidget::quad(qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3, qreal x4, qreal y4)
{
	m_vertices << QVector3D(x1, y1, -0.05f);
	m_vertices << QVector3D(x2, y2, -0.05f);
	m_vertices << QVector3D(x4, y4, -0.05f);

	m_vertices << QVector3D(x3, y3, -0.05f);
	m_vertices << QVector3D(x4, y4, -0.05f);
	m_vertices << QVector3D(x2, y2, -0.05f);

	QVector3D n = QVector3D::normal
	(QVector3D(x2 - x1, y2 - y1, 0.0f), QVector3D(x4 - x1, y4 - y1, 0.0f));

	m_normals << n;
	m_normals << n;
	m_normals << n;

	m_normals << n;
	m_normals << n;
	m_normals << n;

	m_vertices << QVector3D(x4, y4, 0.05f);
	m_vertices << QVector3D(x2, y2, 0.05f);
	m_vertices << QVector3D(x1, y1, 0.05f);

	m_vertices << QVector3D(x2, y2, 0.05f);
	m_vertices << QVector3D(x4, y4, 0.05f);
	m_vertices << QVector3D(x3, y3, 0.05f);

	n = QVector3D::normal
	(QVector3D(x2 - x4, y2 - y4, 0.0f), QVector3D(x1 - x4, y1 - y4, 0.0f));

	m_normals << n;
	m_normals << n;
	m_normals << n;

	m_normals << n;
	m_normals << n;
	m_normals << n;
}

void GLWidget::extrude(qreal x1, qreal y1, qreal x2, qreal y2)
{
	m_vertices << QVector3D(x1, y1, +0.05f);
	m_vertices << QVector3D(x2, y2, +0.05f);
	m_vertices << QVector3D(x1, y1, -0.05f);

	m_vertices << QVector3D(x2, y2, -0.05f);
	m_vertices << QVector3D(x1, y1, -0.05f);
	m_vertices << QVector3D(x2, y2, +0.05f);

	QVector3D n = QVector3D::normal
	(QVector3D(x2 - x1, y2 - y1, 0.0f), QVector3D(0.0f, 0.0f, -0.1f));

	m_normals << n;
	m_normals << n;
	m_normals << n;

	m_normals << n;
	m_normals << n;
	m_normals << n;
}

void GLWidget::TeethSegmentRun(const std::string label_file_path)
{
	//
	//		write your main logic:
	//		calculate PointLabel
	//

	char buffer[256];
	std::fstream outFile;
	outFile.open(label_file_path, std::ios::in);
	//std::cout << label_file_path << "--- all file is as follows:---" << std::endl;
	while (!outFile.eof())
	{
		outFile.getline(buffer, 256, '\n');//getline(char *,int,char) ��ʾ�����ַ��ﵽ256�����������оͽ���
										   //std::cout << buffer << "-" << std::endl;
		if (strcmp(buffer, "background") == 0)
		{
			mm.L.push_back(0);
		}
		else
		{
			mm.L.push_back(1);
		}
	}
	outFile.close();

}
