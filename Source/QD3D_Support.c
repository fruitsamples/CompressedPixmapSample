/****************************//*   	QD3D SUPPORT.C	    *//* By Brian Greenstone      *//****************************//****************************//*    EXTERNALS             *//****************************/#include <NumberFormatting.h>#include <Timer.h>#include <QD3D.h>#include <QD3DGroup.h>#include <QD3DLight.h>#include <QD3DTransform.h>#include <QD3DStorage.h>#include <QD3DMath.h>#include <QD3DErrors.h>#include <ImageCompression.h>#include "myglobals.h"#include "misc.h"#include "qd3d_support.h"extern	EventRecord			gTheEvent;extern	WindowPtr			gModelWindow;extern	Str255			gCompressionRatio,gCompressionSize;/****************************//*    PROTOTYPES            *//****************************/static void CreateDrawContext(QD3DViewDefType *viewDefPtr);static void SetStyles(QD3DStyleDefType *styleDefPtr);static void CreateCamera(QD3DCameraDefType *cameraDefPtr);static void CreateLights(QD3DLightDefType *lightDefPtr);static void CreateView(QD3DSetupInputType *setupDefPtr);static void CreateTexturePixmap(PicHandle pict,unsigned long mapSizeX,							 unsigned long mapSizeY, TQ3CompressedPixmap *bMap);static void TraverseDisplayGroupInfo(TQ3Object theGroup);static void ShowMeshInfo(TQ3Object theMesh);static void ShowAttributeInfo(TQ3AttributeSet theAttribute);static void ShowTriMeshInfo(TQ3Object theTriMesh);/****************************//*    CONSTANTS             *//****************************//*********************//*    VARIABLES      *//*********************/static TQ3CameraObject			gQD3D_CameraObject;static TQ3GroupObject			gQD3D_LightGroup;static TQ3ViewObject			gQD3D_ViewObject;static TQ3DrawContextObject		gQD3D_DrawContext;static TQ3RendererObject		gQD3D_RendererObject;static TQ3ShaderObject			gQD3D_ShaderObject;static	TQ3StyleObject			gQD3D_BackfacingStyle;static	TQ3StyleObject			gQD3D_FillStyle;static	TQ3StyleObject			gQD3D_InterpolationStyle;float	gFramesPerSecond = DEFAULT_FPS;				// this is used to maintain a constant timing velocity as frame rates differ//=======================================================================================================//=============================== VIEW WINDOW SETUP STUFF ===============================================//=======================================================================================================/************** SETUP QD3D WINDOW *******************/void QD3D_SetupWindow(QD3DSetupInputType *setupDefPtr, QD3DSetupOutputType *outputPtr){	CreateView(setupDefPtr);	Q3InteractiveRenderer_SetDoubleBufferBypass(gQD3D_RendererObject,kQ3True);		// let hardware go fast		CreateCamera(&setupDefPtr->camera);										// create new CAMERA object	CreateLights(&setupDefPtr->lights);	SetStyles(&setupDefPtr->styles);						/* DISPOSE OF EXTRA REFERENCES */					Q3Object_Dispose(gQD3D_RendererObject);				// (is contained w/in gQD3D_ViewObject)					/* PASS BACK INFO */					outputPtr->viewObject = gQD3D_ViewObject;	outputPtr->interpolationStyle = gQD3D_InterpolationStyle;	outputPtr->fillStyle = gQD3D_FillStyle;	outputPtr->backfacingStyle = gQD3D_BackfacingStyle;	outputPtr->shaderObject = gQD3D_ShaderObject;	outputPtr->cameraObject = gQD3D_CameraObject;	outputPtr->lightGroup = gQD3D_LightGroup;	outputPtr->drawContext = gQD3D_DrawContext;	outputPtr->window = setupDefPtr->view.displayWindow;		// remember which window	outputPtr->paneClip = setupDefPtr->view.paneClip;}/***************** QD3D_DisposeWindowSetup ***********************///// Disposes of all data created by QD3D_SetupWindow//void QD3D_DisposeWindowSetup(QD3DSetupOutputType *data){	Q3Object_Dispose(data->viewObject);	Q3Object_Dispose(data->interpolationStyle);	Q3Object_Dispose(data->backfacingStyle);	Q3Object_Dispose(data->fillStyle);	Q3Object_Dispose(data->cameraObject);	Q3Object_Dispose(data->lightGroup);	Q3Object_Dispose(data->drawContext);	Q3Object_Dispose(data->shaderObject);}/******************* CREATE GAME VIEW *************************/static void CreateView(QD3DSetupInputType *setupDefPtr){TQ3Status	myErr;unsigned long	hints;				/* CREATE NEW VIEW OBJECT */					gQD3D_ViewObject = Q3View_New();	if (gQD3D_ViewObject == nil)		DoFatalAlert("\pQ3View_New failed!");			/* CREATE & SET DRAW CONTEXT */		CreateDrawContext(&setupDefPtr->view); 								// init draw context		myErr = Q3View_SetDrawContext(gQD3D_ViewObject, gQD3D_DrawContext);			// assign context to view	if (myErr == kQ3Failure)		DoFatalAlert("\pQ3View_SetDrawContext Failed!");			/* CREATE & SET RENDERER */	gQD3D_RendererObject = Q3Renderer_NewFromType(setupDefPtr->view.rendererType);	// create new RENDERER object	if (gQD3D_RendererObject == nil)	{		QD3D_ShowError("\pQ3Renderer_NewFromType Failed!", true);		CleanQuit();	}	myErr = Q3View_SetRenderer(gQD3D_ViewObject, gQD3D_RendererObject);				// assign renderer to view	if (myErr == kQ3Failure)		DoFatalAlert("\pQ3View_SetRenderer Failed!");		/* SET RENDERER FEATURES */			Q3InteractiveRenderer_GetRAVEContextHints(gQD3D_RendererObject, &hints);	hints &= ~kQAContext_NoZBuffer; 				// Z buffer is on 	hints &= ~kQAContext_DeepZ; 					// Z buffer is not deep, ergo it's shallow 	hints |= kQAContext_NoDither; 					// No Dither 	Q3InteractiveRenderer_SetRAVEContextHints(gQD3D_RendererObject, hints);			Q3InteractiveRenderer_SetRAVETextureFilter(gQD3D_RendererObject,kQATextureFilter_Best);		Q3InteractiveRenderer_SetDoubleBufferBypass(gQD3D_RendererObject,kQ3True);}/**************** CREATE SKELETON DRAW CONTEXT *********************/static void CreateDrawContext(QD3DViewDefType *viewDefPtr){TQ3DrawContextData		drawContexData;TQ3MacDrawContextData	myMacDrawContextData;Rect					r;	r = viewDefPtr->displayWindow->portRect;			/* FILL IN DRAW CONTEXT DATA */	drawContexData.clearImageMethod = kQ3ClearMethodWithColor;				// how to clear	drawContexData.clearImageColor = viewDefPtr->clearColor;				// color to clear to	drawContexData.pane.min.x = r.left+viewDefPtr->paneClip.left;			// set bounds?	drawContexData.pane.max.x = r.right-viewDefPtr->paneClip.right;	drawContexData.pane.min.y = r.top+viewDefPtr->paneClip.top;	drawContexData.pane.max.y = r.bottom-viewDefPtr->paneClip.bottom;	drawContexData.paneState = kQ3True;										// use bounds?	drawContexData.maskState = kQ3False;									// no mask	drawContexData.doubleBufferState = kQ3True;								// double buffering	myMacDrawContextData.drawContextData = drawContexData;					// set MAC specifics	myMacDrawContextData.window = (CWindowPtr)viewDefPtr->displayWindow;	// assign window to draw to	myMacDrawContextData.library = kQ3Mac2DLibraryNone;						// use standard QD libraries (no GX crap!)	myMacDrawContextData.viewPort = nil;									// (for GX only)	myMacDrawContextData.grafPort = (CWindowPtr)viewDefPtr->displayWindow;	// assign grafport			/* CREATE DRAW CONTEXT */	gQD3D_DrawContext = Q3MacDrawContext_New(&myMacDrawContextData);	if (gQD3D_DrawContext == nil)		DoFatalAlert("\pQ3MacDrawContext_New Failed!");}/**************** SET STYLES ****************///// Creates style objects which define how the scene is to be rendered.// It also sets the shader object.//static void SetStyles(QD3DStyleDefType *styleDefPtr){				/* SET INTERPOLATION (FOR SHADING) */						gQD3D_InterpolationStyle = Q3InterpolationStyle_New(styleDefPtr->interpolation);	if (gQD3D_InterpolationStyle == nil)		DoFatalAlert("\pQ3InterpolationStyle_New Failed!");					/* SET BACKFACING */	gQD3D_BackfacingStyle = Q3BackfacingStyle_New(styleDefPtr->backfacing);	if (gQD3D_BackfacingStyle == nil )		DoFatalAlert("\pQ3BackfacingStyle_New Failed!");				/* SET POLYGON FILL STYLE */							gQD3D_FillStyle = Q3FillStyle_New(styleDefPtr->fill);	if ( gQD3D_FillStyle == nil )		DoFatalAlert("\p Q3FillStyle_New Failed!");					/* SET THE SHADER TO USE */	switch(styleDefPtr->illuminationType)	{		case	kQ3IlluminationTypePhong:				gQD3D_ShaderObject = Q3PhongIllumination_New();				if ( gQD3D_ShaderObject == nil )					DoFatalAlert("\p Q3PhongIllumination_New Failed!");				break;						case	kQ3IlluminationTypeLambert:				gQD3D_ShaderObject = Q3LambertIllumination_New();				if ( gQD3D_ShaderObject == nil )					DoFatalAlert("\p Q3LambertIllumination_New Failed!");				break;						case	kQ3IlluminationTypeNULL:				gQD3D_ShaderObject = Q3NULLIllumination_New();				if ( gQD3D_ShaderObject == nil )					DoFatalAlert("\p Q3NullIllumination_New Failed!");				break;	}	}/****************** CREATE CAMERA *********************/static void CreateCamera(QD3DCameraDefType *cameraDefPtr){TQ3CameraData					myCameraData;TQ3ViewAngleAspectCameraData	myViewAngleCameraData;TQ3Area							pane;TQ3Status						status;TQ3Status	myErr;	status = Q3DrawContext_GetPane(gQD3D_DrawContext,&pane);				// get window pane info	if (status == kQ3Failure)		DoFatalAlert("\pQ3DrawContext_GetPane Failed!");				/* FILL IN CAMERA DATA */					myCameraData.placement.cameraLocation = cameraDefPtr->from;			// set camera coords	myCameraData.placement.pointOfInterest = cameraDefPtr->to;			// set target coords	myCameraData.placement.upVector = cameraDefPtr->up;					// set a vector that's "up"	myCameraData.range.hither = cameraDefPtr->hither;					// set frontmost Z dist	myCameraData.range.yon = cameraDefPtr->yon;							// set farthest Z dist	myCameraData.viewPort.origin.x = -1.0;								// set view origins?	myCameraData.viewPort.origin.y = 1.0;	myCameraData.viewPort.width = 2.0;	myCameraData.viewPort.height = 2.0;	myViewAngleCameraData.cameraData = myCameraData;	myViewAngleCameraData.fov = cameraDefPtr->fov;						// larger = more fisheyed	myViewAngleCameraData.aspectRatioXToY =				(pane.max.x-pane.min.x)/(pane.max.y-pane.min.y);	gQD3D_CameraObject = Q3ViewAngleAspectCamera_New(&myViewAngleCameraData);	 // create new camera	if (gQD3D_CameraObject == nil)		DoFatalAlert("\pQ3ViewAngleAspectCamera_New failed!");			myErr = Q3View_SetCamera(gQD3D_ViewObject, gQD3D_CameraObject);		// assign camera to view	if (myErr == kQ3Failure)		DoFatalAlert("\pQ3View_SetCamera Failed!");}/********************* CREATE LIGHTS ************************/static void CreateLights(QD3DLightDefType *lightDefPtr){TQ3GroupPosition		myGroupPosition;TQ3LightData			myLightData;TQ3DirectionalLightData	myDirectionalLightData;TQ3LightObject			myLight;short					i;TQ3Status	myErr;			/* CREATE NEW LIGHT GROUP */				gQD3D_LightGroup = Q3LightGroup_New();						// make new light group	if ( gQD3D_LightGroup == nil )		DoFatalAlert("\p Q3LightGroup_New Failed!");	myLightData.isOn = kQ3True;									// light is ON				/************************/			/* CREATE AMBIENT LIGHT */			/************************/	if (lightDefPtr->ambientBrightness != 0)						// see if ambient exists	{		myLightData.color = lightDefPtr->ambientColor;				// set color of light		myLightData.brightness = lightDefPtr->ambientBrightness;	// set brightness value		myLight = Q3AmbientLight_New(&myLightData);					// make it		if ( myLight == nil )			DoFatalAlert("\pQ3AmbientLight_New Failed!");		myGroupPosition = Q3Group_AddObject(gQD3D_LightGroup, myLight);	// add to group		if ( myGroupPosition == 0 )			DoFatalAlert("\p Q3Group_AddObject Failed!");		Q3Object_Dispose(myLight);									// dispose of light	}			/**********************/			/* CREATE FILL LIGHTS */			/**********************/				for (i=0; i < lightDefPtr->numFillLights; i++)	{				myLightData.color = lightDefPtr->fillColor[i];						// set color of light		myLightData.brightness = lightDefPtr->fillBrightness[i];			// set brightness		myDirectionalLightData.lightData = myLightData;						// refer to general light info		myDirectionalLightData.castsShadows = kQ3False;						// no shadows		myDirectionalLightData.direction =  lightDefPtr->fillDirection[i];	// set fill vector		myLight = Q3DirectionalLight_New(&myDirectionalLightData);			// make it		if ( myLight == nil )			DoFatalAlert("\p Q3DirectionalLight_New Failed!");		myGroupPosition = Q3Group_AddObject(gQD3D_LightGroup, myLight);		// add to group		if ( myGroupPosition == 0 )			DoFatalAlert("\p Q3Group_AddObject Failed!");		Q3Object_Dispose(myLight);											// dispose of light	}				/* ASSIGN LIGHT GROUP TO VIEW */				myErr = Q3View_SetLightGroup(gQD3D_ViewObject, gQD3D_LightGroup);		// assign light group to view	if (myErr == kQ3Failure)		DoFatalAlert("\pQ3View_SetLightGroup Failed!");		}/******************** QD3D CHANGE DRAW SIZE *********************///// Changes size of stuff to fit new window size.//void QD3D_ChangeDrawSize(QD3DSetupOutputType *setupInfo){Rect			r;TQ3Area			pane;TQ3ViewAngleAspectCameraData	cameraData;			/* CHANGE DRAW CONTEXT PANE SIZE */				r = setupInfo->window->portRect;							// get size of window	pane.min.x = r.left+setupInfo->paneClip.left;										// set pane size	pane.max.x = r.right-setupInfo->paneClip.right;	pane.min.y = r.top+setupInfo->paneClip.top;	pane.max.y = r.bottom-setupInfo->paneClip.bottom;	Q3DrawContext_SetPane(setupInfo->drawContext,&pane);		// update pane in draw context				/* CHANGE CAMERA ASPECT RATIO */					Q3ViewAngleAspectCamera_GetData(setupInfo->cameraObject,&cameraData);			// get camera data	cameraData.aspectRatioXToY = (pane.max.x-pane.min.x)/(pane.max.y-pane.min.y);	// set new aspect ratio	Q3ViewAngleAspectCamera_SetData(setupInfo->cameraObject,&cameraData);			// set new camera data}/******************* QD3D DRAW SCENE *********************/void QD3D_DrawScene(QD3DSetupOutputType *setupInfo, void (*drawRoutine)(QD3DSetupOutputType *)){TQ3Status				myStatus;TQ3ViewStatus			myViewStatus;			/* START RENDERING *///startTrace("test.tt6");	myStatus = Q3View_StartRendering(setupInfo->viewObject);				if ( myStatus == kQ3Failure )	{		DoFatalAlert("\p Q3View_StartRendering Failed!");	}				/***************/			/* RENDER LOOP */			/***************/	do	{				/* DRAW STYLES */						myStatus = Q3Style_Submit(setupInfo->interpolationStyle,setupInfo->viewObject);		if ( myStatus == kQ3Failure )			DoFatalAlert("\p Q3Style_Submit Failed!");					myStatus = Q3Style_Submit(setupInfo->backfacingStyle,setupInfo->viewObject);		if ( myStatus == kQ3Failure )			DoFatalAlert("\p Q3Style_Submit Failed!");					myStatus = Q3Style_Submit(setupInfo->fillStyle, setupInfo->viewObject);		if ( myStatus == kQ3Failure )			DoFatalAlert("\p Q3Style_Submit Failed!");		myStatus = Q3Shader_Submit(setupInfo->shaderObject, setupInfo->viewObject);		if ( myStatus == kQ3Failure )			DoFatalAlert("\p Q3Shader_Submit Failed!");			/* CALL INPUT DRAW FUNCTION */		drawRoutine(setupInfo);		myViewStatus = Q3View_EndRendering(setupInfo->viewObject);			} while ( myViewStatus == kQ3ViewStatusRetraverse );	//stopTrace();//CleanQuit();}//=======================================================================================================//=============================== MISC ==================================================================//=======================================================================================================/************** QD3D CALC FRAMES PER SECOND *****************/float	QD3D_CalcFramesPerSecond(void){UnsignedWide	wide;unsigned long	now;static	unsigned long then = 0;	Microseconds(&wide);	now = wide.lo;	if (then != 0)	{		gFramesPerSecond = (float)1000000.0/(float)(now-then);		if (gFramesPerSecond < DEFAULT_FPS)			// (avoid divide by 0's later)			gFramesPerSecond = DEFAULT_FPS;	//		if (gTheEvent.modifiers & shiftKey)		{			SetPort(gModelWindow);			ForeColor(yellowColor);			BackColor(blackColor);			TextMode(srcCopy);			TextSize(12);			TextFace(bold);			MoveTo(10,13);			DrawString("\pCompression % = ");			DrawString(gCompressionRatio);			DrawString("\p    #Bytes = ");			DrawString(gCompressionSize);			DrawString("\p             ");		}	}	else		gFramesPerSecond = DEFAULT_FPS;			then = now;								// remember time			return(gFramesPerSecond);}#pragma mark ========== error stuff ===========/******************* QD3D: SHOW ERROR *************************///// Returns true if Error, false if just a warning.//Boolean QD3D_ShowError(Str255 errString, Boolean showWarnings){TQ3Error	err;TQ3Warning	warning;Str255		numStr;		/* DO ERRORS */			err = Q3Error_Get(nil);	if (err != 0)	{		DoAlert(errString);		switch(err)		{			case	kQ3ErrorViewNotStarted:					DoFatalAlert("\pError:kQ3ErrorViewNotStarted");					break;								case	kQ3ErrorOutOfMemory:					DoFatalAlert("\pError:kQ3ErrorOutOfMemory");					break;								default:					ShowSystemErr(err);		}		return(true);	}			/* DO WARNINGS */	else	{		if (!showWarnings)			return(false);				DoAlert(errString);		warning = Q3Warning_Get(nil);		switch(warning)		{			case	kQ3WarningFunctionalityNotSupported:					DoAlert("\pWarning: kQ3WarningFunctionalityNotSupported");					break;								default:					NumToString(err, numStr);					DoAlert (numStr);		}		return(false);	}		}/************ QD3D: SHOW RECENT ERROR *******************/void QD3D_ShowRecentError(void){TQ3Error	q3Err;Str255		s;		q3Err = Q3Error_Get(nil);	if (q3Err == kQ3ErrorOutOfMemory)		QD3D_DoMemoryError();	else	if (q3Err == kQ3ErrorMacintoshError)		DoFatalAlert("\pkQ3ErrorMacintoshError");	else	if (q3Err != 0)	{		NumToString(q3Err,s);		DoFatalAlert(s);	}}/***************** QD3D: DO MEMORY ERROR **********************/void QD3D_DoMemoryError(void){	InitCursor();	NoteAlert(129,nil);	CleanQuit();}