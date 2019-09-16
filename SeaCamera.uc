class SeaCamera extends Camera
	config(Game)
	dependson(SeaPlayerControllerFight)
	native;

//用于trace,这个值满足足够大就可以
const MAX_TRACE_RADIUS = 90000000;


//add by yeleiyu@icee.cn for 狙击视角
var PostProcessChain TelescopeViewPPC;
//add by yeleiyu@icee.cn for 狙击视角
var array<PostProcessChain> OldPostProcessChain;

//add by yeleiyu@icee.cn for 下雨功能
var ParticleSystem RainFX;
var ParticleSystemComponent		RainEffect;
var globalconfig string  SmallRainName;
var globalconfig string  HeavyRainName;
var globalconfig string  SnowName;
var globalconfig string  HeavySnowName;
var globalconfig float  EffectDistance2Cam;
var globalconfig float  EffectScale;
var int RainState;
var bool PreEffectEnable;

var MaterialEffect TelescopeViewEffect;
var transient MaterialInstanceConstant TelescopeViewMaterialInstance;
var linearcolor Resolution;
var bool IsResolutionChange;




//目前武器支持的视角在配置中用一个bit位方式的值标记，策划配置表中使用
const FirstPerson	= 1;
const ThirdPerson	= 2;
const Telescope		= 4;
const ArtilleryEye	= 8;
const FightJetEye	= 16;
const BirdEye		= 32;
const FreeCamera	= 64;

/** 上一个相机视角 */
var name _PrevCameraStyle;

var globalconfig float		_ToleranceDistance;
/** 相机优先切换模式，如果武器切到了当前视角不支持的情况，自动根据这个列表寻找高优先级推荐视角*/
var globalconfig array<INT> _PriorityCamSwitchMode;

// 鹰眼视角调整Listener的位置
var globalconfig float _ListenerHeightScale;
var globalconfig float _ListenerHeightOffset;

// 目标锁定功能
var SeaPawn						_LockedPawn;
var int							_LockedDeltaYaw;

var bool						_bEnableBlend;
var TPOV						_BlendedStartPOV;			// 过渡起始POV
var ViewTargetTransitionParams	_BlendParamsWhenSwitch;
var ViewTargetTransitionParams	_BlendParamsWhenFollow;
var float						_BlendTimeWhenSwitch;
var float						_BlendTimeWhenFollow;
var bool						_bAlwaysLookAt;				// camera切换时是否需要始终看着某处
var vector						_PointLookAt;				// LookAt点，_bAlwaysLookAt==true时有效

var vector2D					_MouseDelta;				// mouse offset
var vector2D					_MouseDeltaByDrag;
var vector2D					_MinBound;					// 最小边界值
var vector2D					_MaxBound;					// 最大边界值
var	float						_MaxSceneHeight;			// 场景最大高度
var float						_OceanZ;					// 海平面高度值

var bool						_bUseSpecifiedLookAtPoint;	// 视角切换时使用指定LookPoint
var vector						_SpecifiedLookAtPoint;		// 指定LookAtPoint
var bool						_bUseSpecifiedLookAtPointWhenFollow;	// 状态内过渡时盯着指定点
var vector						_SpecifiedLookAtPointWhenFollow;

////// 第一人称视角参数
struct native FirstViewParam
{
	var float		_CustomizedFirstFOV;			// 第一人称FOV
	var float		_FirstPersonViewOffset;			// 第一人称视角POV拉高比率
	var ViewTargetTransitionParams	_BlendParams;	// 切换至第一人称视角时的过渡参数
};
var globalconfig FirstViewParam		_FirstViewParam;


////// 第三人称视角参数
struct native FreeViewParam
{
	var float			_DefaultFOV;
	var float			_Ratio;						// 短半轴长与长半轴长比率
	var array<float>    _ScaleExpandLongAxle;		// 椭圆长半轴长度与配置的长半轴长度比值
    var float		    _ViewPosHeightRatio;        // 观察点高度比率
	var float			_LimitPitchMax;				// 视角PITCH限制， in degree
	var float			_LimitPitchMin;				// 视角PITCH限制， in degree
	var float			_FixLength;                 // 固定长轴长度，如果不为0，就根据这个值而不是实际船长配置算长轴
	var float			_RejectParam;				// Camera反弹参数，目前仅大厅内第三人称使用
	var ViewTargetTransitionParams	_BlendParams;	// 切换至第三人称视角时的过渡参数
	var ViewTargetTransitionParams	_InternalBlendParams;	// 状态内切换档位时的过渡参数

	var array<float>	_ScaleLevelList;			// 每档基于最长半轴的比例
	var	float			_BasicHalfLongAxle;			// 基础长半轴长度	_BasicHalfLongAxle = 0.5f * Length * _ScaleExpandLongAxle;
	var	float			_BasicHalfShortAxle;		// 基础短半轴长度	_BasicHalfShortAxle = _BasicHalfLongAxle * _Ratio;
	var int				_CurAxleLevel;              // 当前使用哪一档的_ScaleExpandLongAxle
	var float			_CurScaleAlpha;
	var float			_StartScaleAlpha;			// 
	var float			_PendingScaleAlpha;
	var float			_ScaleAlpha;				// 档位过渡时的参数,[0, 1]
};
var globalconfig FreeViewParam	_FreeViewParam;		// 战斗内第三人称视角参数
var globalconfig FreeViewParam	_FreeViewParamEx;	// 大厅观察舰船时第三人称视角参数



/////// 望远镜视角参数
struct native TelescopeViewParam
{
	var array<float>				_TelescopeViewOffset;				// 狙镜视角POV拉高比率
	var array<float>				_TelescopeViewOffsetExceptCannon;	// 非火炮武器时狙镜视角POV拉高比率
	var array<float>				_TelescopeZoomLevel;				// 每档缩放fov值
	var ViewTargetTransitionParams	_BlendParams;						// 切换至望远镜视角时的过渡参数
	var ViewTargetTransitionParams	_InternalBlendParams;				// 状态内切换档位时的过渡参数

	var bool						_bUseCannon;
	var int							_CurrTelescopeLevel;				// 当前望远镜缩放等级
	var float						_CurScaleAlpha;
	var float						_CurrViewOffset;
};
var globalconfig TelescopeViewParam _TelescopeViewParam;



////// 火炮视角参数
struct native ArtilleryViewParam
{
	var float		_DefaultFOV;
	var Rotator		_DefaultCamRotation;			// 相机默认转向
	var float		_MaxZoomOut;					// 最大缩放
	var float		_MinZoomOut;					// 最小缩放
	var float		_DefaultZoomOut;				// 默认缩放
	var float		_MaxZoomOutCached;				// 最大缩放缓存，仅舰载机鹰眼视角使用
	var float		_MinZoomOutCached;				// 最小缩放缓存，仅舰载机鹰眼视角使用
	var float		_DefaultZoomOutCached;			// 默认缩放缓存，仅舰载机鹰眼视角使用
	var float		_CurrentZoomOut;
	var float		_EagleEyeMoveSpeed;				// 鹰眼视角下鼠标平移速度
	var float		_EagleEyeZoomSpeed;             // 鹰眼视角下鼠标缩放速度
	var ViewTargetTransitionParams	_BlendParams;	// 切换至火炮视角时的过渡参数
	var ViewTargetTransitionParams	_InternalBlendParams;	// SetViewTargetPos时的过渡参数
	var ViewTargetTransitionParams	_InternalBlendParams2;	// ZOOM IN/ZOOM OUT时的过渡参数
};
var globalconfig ArtilleryViewParam _ArtilleryViewParam;     //火炮鹰眼视角所需参数
var globalconfig ArtilleryViewParam _FightJetViewParam;      //舰载机鹰眼视角所需参数 根据策划需求，舰载机与火炮鹰眼视角只是配置参数上的差异
var globalconfig ArtilleryViewParam _ObserverViewParam;		 //观察者鹰眼视角所需参数



////// 鸟瞰模式
struct native BirdViewParam
{
	var float			_MinAngle;					// 与水平面的最小夹角
	var float			_MaxAngle;					// 与水平面的最大夹角
	var float			_MaxHeight;					// camera的最高高度（世界空间）
	var float			_ScaleOfAscend;				// 上升时的速率，基于aMouseY
	var float			_ScaleOfBeyond;				// 到达最高高度后继续前推时的速率，基于aMouseY
	var array<float>	_BirdZoomLevel;				// 每档缩放fov值
	var int				_CurrBirdLevel;				// 当前望远镜缩放等级
	var ViewTargetTransitionParams	_BlendParams;	// 切换至鸟瞰模式时的过渡参数
	var ViewTargetTransitionParams	_InternalBlendParams;	// ZOOM IN/ZOOM OUT时的过渡参数
	var ViewTargetTransitionParams	_InternalBlendParams2;	// SetViewTargetPos3D时的过渡参数

	var float			_DistanceXY1;				// camera无位移且与水平面夹角为_MinAngle时，相对camera在XY平面的距离
	var float			_DistanceXY2;				// camera上升到最高高度且与水平面夹角为_MinAngle时，相对camera在XY平面的距离
	var float			_DistanceXY3;				// 最远射程相对camera初始位置的距离
	var int				_PointInWhere;				// 1，[0, _BirdViewParam._DistanceXY1]；
													// 2，[_BirdViewParam._DistanceXY1, _BirdViewParam._DistanceXY2]
													// 3, [_BirdViewParam._DistanceXY2, _BirdViewParam._DistanceXY3]
	var float			_DistanceXY;				// LookAt点与ViewTarget.Location在XY平面的距离
};
var globalconfig BirdViewParam		_BirdViewParam;

////// 编辑模式
struct native EditorModeParam
{
	var float		_DefaultFOV;                    //  默认FOV
    var float       _MinFOV;                        //  最小FOV
    var float       _MaxFOV;                        //  最大FOV
    var int         _TranslateRatio;                //  移动转换速率
    var int         _MinTransRatio;                 //  最小移动转换速率
    var int         _MaxTransRatio;                 //  最大移动转换速率
    var int         _RotateRatio;                   //  旋转速度
    var int         _MinRotRatio;                   //  最小旋转速度
    var int         _MaxRotRatio;                   //  最大旋转速度
    var int         _ZoomRatio;                     //  FOV缩放速率
    var bool        _AutoReverseFOV;                //  自动恢复默认FOV
    var float       _Distance;
    var int         _FocusPartType;
    var int         _FocusPartIndex;
    var Pawn        _TraceTarget;
};
var globalconfig EditorModeParam _EditorModeParam;

/** 编辑器模式Camera操作标记*/
var int _RadialMoveFlag;
var int _SideMoveFlag;
var int _VerticalMoveFlag;
var int _FOVZoomFlag;

////// 炮弹追踪视角
struct native ProjectileViewParam
{
    var float       _FOV;
    var float       _InitPitch;
    var float       _Distance;
    var float       _InterpSpeed;
    var Actor       _TraceTarget;
    var vector      _LastTracePos;
};
var globalconfig ProjectileViewParam _ProjectileViewParam;

enum ECameraShakeLevel
{
	/** 轻微*/
	ECSL_LITTLE,
	/** 中等*/
	ECSL_MIDDLE,
	/** 剧烈*/
	ECSL_LARGE,
    /** 射击后坐大*/
    ECSL_FIRE_EXTREME,
    /** 射击后坐小*/
    ECSL_FIRE_NORMAL
};
/** 相机震动动画*/
var array<CameraAnim> _ShakeCameraAnim;
/** 鹰眼视角受击提示*/
var CameraAnim _EagleEyeHurt;
/** 开炮震动动画*/
var array<CameraAnim> _FireRecoilCameraAnim;
/** 舰载机编队投影距离Camera的有效距离*/
var globalconfig INT _SquadShadowValidDistance;

// 鹰眼视角下的背景音效
var globalconfig String	_JetViewSoundURL;
var SoundCue	        _JetViewSoundCue;
var AudioComponent      _JetViewSound;

//add by yeleiyu@icee.cn for 狙击视角
native function GetCurResolution();
native function InitCurResolution();

//add by yeleiyu@icee.cn for 狙击视角
private function NotifyBindPostProcessEffects()
{
	local InstanceDesc InstDesc;
	if( `ClientDataStore.FindInstanceDesc(`ClientWorldInfo.GetFightMapID(), InstDesc) )
	{
			TelescopeViewPPC = PostProcessChain(DynamicLoadObject(InstDesc.TelescopePostProcessChain, class'PostProcessChain'));

	}
	else
	{
			TelescopeViewPPC = PostProcessChain(DynamicLoadObject("A_Aircraft.Demage.SSAO_Chain_TelescopeView", class'PostProcessChain'));

	}

	InitCurResolution();
	// create hit effect material instance
	TelescopeViewEffect = MaterialEffect(TelescopeViewPPC.FindPostProcessEffect('TelescopeView'));
	if (TelescopeViewEffect != None)
	{
		if (MaterialInstanceConstant(TelescopeViewEffect.Material) != None )
		{
			// the runtime material already exists; grab it
			TelescopeViewMaterialInstance = MaterialInstanceConstant(TelescopeViewEffect.Material);
		}
		else
		{
			TelescopeViewMaterialInstance = new(TelescopeViewEffect) class'MaterialInstanceConstant';
			TelescopeViewMaterialInstance.SetParent(TelescopeViewEffect.Material);
			TelescopeViewEffect.Material = TelescopeViewMaterialInstance;
		}
		TelescopeViewMaterialInstance.SetVectorParameterValue('little leaves', Resolution);
	}
}
//add by yeleiyu@icee.cn for 下雨功能
function InitializeFor(PlayerController PC)
{
	//add by yeleiyu@icee.cn for 下雨功能
	local InstanceDesc InstDesc;
	local SeaMapInfo	SMInfo;

	RainState = 0;
	PreEffectEnable=`GameOptions._VideoSetting.WeatherEffectEnable;
	if( `ClientDataStore.FindInstanceDesc(`ClientWorldInfo.GetFightMapID(), InstDesc) )
	{
		RainState = InstDesc.RainState;
	}
	if( RainState > 0 )
	{		
		if( RainState == 1 )
		{
			RainFX=ParticleSystem(DynamicLoadObject(SmallRainName, class'ParticleSystem'));
		}
		else if( RainState == 2 )
		{
			RainFX=ParticleSystem(DynamicLoadObject(HeavyRainName, class'ParticleSystem'));
		}
		else if( RainState == 3 )
		{
			RainFX=ParticleSystem(DynamicLoadObject(SnowName, class'ParticleSystem'));
		}
		else if( RainState == 4 )
		{
			RainFX=ParticleSystem(DynamicLoadObject(HeavySnowName, class'ParticleSystem'));
		}
		RainEffect = new(self) class'ParticleSystemComponent';
		RainEffect.SetTemplate(RainFX);
		RainEffect.SetActorCollision(FALSE, FALSE);
		RainEffect.SetAbsolute(TRUE,TRUE,TRUE);
		RainEffect.SetScale(EffectScale);
		RainEffect.SetRotation(rot(0, 0, 0));
		RainEffect.SetTranslation(vect(0,0,0));
		AttachComponent(RainEffect);
		if(`GameOptions._VideoSetting.WeatherEffectEnable && RainState > 0)
		{		
			RainEffect.SetActive(true);
			RainEffect.SetHidden(FALSE);
		}
		else if(!`GameOptions._VideoSetting.WeatherEffectEnable && RainState > 0)
		{
			RainEffect.SetActive(false);
			RainEffect.SetHidden(TRUE);
		}


	}

	NotifyBindPostProcessEffects();

	// 修改默认第三人称时的FOV
	DefaultFOV = _FreeViewParam._DefaultFOV;

	SMInfo = SeaMapInfo(WorldInfo.GetMapInfo());
	_MinBound = SMInfo._Mix;
	_MaxBound = SMInfo._Max;

	_ToleranceDistance = Abs(_ToleranceDistance);
	
	Super.InitializeFor(PC);
}

function EnableCameraBlend(bool bEnable)
{
	_bEnableBlend = bEnable;
}

simulated event UpdateCamera(float DeltaTime)
{
	if (`SeaUIController != none )
		`SeaUIController.Tick(DeltaTime, SeaPlayerControllerFight(PCOwner));

	if ( PCOwner.IsLocalPlayerController() || !bUseClientSideCameraUpdates || bDebugClientSideCamera )
	{
		DoUpdateCamera(DeltaTime);
	}
}

simulated function DoUpdateCamera(float DeltaTime)
{
	local float DurationPct;
	local float BlendPct;
	local TPOV NewPOV;
	local bool bProcessIt;
	local vector ViewLoc;
	// 更新目标位置
	UpdateViewTarget(ViewTarget, DeltaTime);
	NewPOV = ViewTarget.POV;

	bProcessIt = false;
	if( _BlendTimeWhenSwitch > 0.0f )
	{
		_BlendTimeWhenSwitch -= DeltaTime;
		if( _BlendTimeWhenSwitch <= DeltaTime )
		{
			_BlendTimeWhenSwitch = 0.f;

			BlendPct = 1.f;

			EndCameraSwitch();
		}
		else
		{
			DurationPct = (_BlendParamsWhenSwitch.BlendTime - _BlendTimeWhenSwitch) / _BlendParamsWhenSwitch.BlendTime;

			switch (_BlendParamsWhenSwitch.BlendFunction)
			{
			case VTBlend_Linear:
				BlendPct = Lerp(0.f, 1.f, DurationPct);
				break;
			case VTBlend_Cubic:
				BlendPct = FCubicInterp(0.f, 0.f, 1.f, 0.f, DurationPct);
				break;
			case VTBlend_EaseIn:
				BlendPct = FInterpEaseIn(0.f, 1.f, DurationPct, _BlendParamsWhenSwitch.BlendExp);
				break;
			case VTBlend_EaseOut:
				BlendPct = FInterpEaseOut(0.f, 1.f, DurationPct, _BlendParamsWhenSwitch.BlendExp);
				break;
			case VTBlend_EaseInOut:
				BlendPct = FInterpEaseInOut(0.f, 1.f, DurationPct, _BlendParamsWhenSwitch.BlendExp);
				break;
			}
		}

		NewPOV = BlendViewTargetOfSwitch(_BlendedStartPOV, ViewTarget.POV, BlendPct);

		bProcessIt = true;
	}

	// step 2
	if( !bProcessIt && _BlendTimeWhenFollow > 0.0f )
	{
		_BlendTimeWhenFollow -= DeltaTime;
		if( _BlendTimeWhenFollow <= DeltaTime )
		{
			_BlendTimeWhenFollow = 0.0f;

			BlendPct = 1.f;

			EndCameraFollow();
		}
		else
		{
			DurationPct = (_BlendParamsWhenFollow.BlendTime - _BlendTimeWhenFollow) / _BlendParamsWhenFollow.BlendTime;

			switch (_BlendParamsWhenFollow.BlendFunction)
			{
			case VTBlend_Linear:
				BlendPct = Lerp(0.f, 1.f, DurationPct);
				break;
			case VTBlend_Cubic:
				BlendPct = FCubicInterp(0.f, 0.f, 1.f, 0.f, DurationPct);
				break;
			case VTBlend_EaseIn:
				BlendPct = FInterpEaseIn(0.f, 1.f, DurationPct, _BlendParamsWhenFollow.BlendExp);
				break;
			case VTBlend_EaseOut:
				BlendPct = FInterpEaseOut(0.f, 1.f, DurationPct, _BlendParamsWhenFollow.BlendExp);
				break;
			case VTBlend_EaseInOut:
				BlendPct = FInterpEaseInOut(0.f, 1.f, DurationPct, _BlendParamsWhenFollow.BlendExp);
				break;
			}
		}

		NewPOV = BlendViewTargetOfFollow(_BlendedStartPOV, ViewTarget.POV, BlendPct);

		bProcessIt = true;
	}


	//add by yeleiyu@icee.cn for 下雨功能
	if(`GameOptions._VideoSetting.WeatherEffectEnable && RainState > 0)
	{	
		if(`GameOptions._VideoSetting.WeatherEffectEnable != PreEffectEnable)
		{
			PreEffectEnable = `GameOptions._VideoSetting.WeatherEffectEnable;
			RainEffect.SetHidden(false);
			RainEffect.SetActive(true);
		}

		if(SeaPlayerControllerFight(PCOwner)!=none)
		{
			ViewLoc = SeaPlayerControllerFight(PCOwner)._CameraLocation + SeaPlayerControllerFight(PCOwner)._CameraDir * EffectDistance2Cam;
			RainEffect.SetTranslation(ViewLoc);
		}
	}
	else if(!`GameOptions._VideoSetting.WeatherEffectEnable && RainState > 0)
	{
		if(`GameOptions._VideoSetting.WeatherEffectEnable != PreEffectEnable)
		{
			PreEffectEnable = `GameOptions._VideoSetting.WeatherEffectEnable;
		}
		RainEffect.SetActive(false);
		RainEffect.SetHidden(TRUE);
	}


	FillCameraCache(NewPOV);
}

function TPOV BlendViewTargetOfFollow(const out TPOV A,const out TPOV B, float Alpha)
{
	local TPOV	POV;

	POV.Location	= VLerp(A.Location, B.Location, Alpha);
	POV.FOV			= Lerp(A.FOV, B.FOV, Alpha);
	POV.Rotation	= RLerp(A.Rotation, B.Rotation, Alpha, TRUE);

	return POV;
}

function TPOV BlendViewTargetOfSwitch(const out TPOV A,const out TPOV B, float Alpha)
{
	local TPOV	POV;
	local rotator NewRot;
	local rotator TempB;
	local vector Z1, Z2;
	local float DeltaYaw;
	local vector Axis;
	local Quat DeltaQuat;

	POV.Location	= VLerp(A.Location, B.Location, Alpha);
	POV.FOV			= Lerp(A.FOV, B.FOV, Alpha);

	if( _bAlwaysLookAt )
	{
		TempB	= RInterpNormalRotationTo(A.Rotation, Normal(Vector(B.Rotation)), 1.f);
				
		Z1 = Normal(MatrixGetAxis(MakeRotationMatrix(TempB), AXIS_Z));
		Z2 = Normal(MatrixGetAxis(MakeRotationMatrix(B.Rotation), AXIS_Z));
		DeltaYaw = ACos(Z1 dot Z2);
		Axis = Z1 Cross Z2;
		if( (Axis Dot Vector(B.Rotation)) < 0 )
		{
			DeltaYaw *= -1.f;
		}
		NewRot = RInterpNormalRotationTo(A.Rotation, Normal(_PointLookAt - POV.Location), 1.f);

		DeltaQuat = QuatFromAxisAndAngle(Normal(Vector(NewRot)), DeltaYaw * Alpha);

		POV.Rotation = Normalize(MatrixGetRotator(MakeRotationMatrix(NewRot) * MakeRotationMatrix(QuatToRotator(DeltaQuat))));
		//`log("StartRot: "@Normalize(A.Rotation)@"	EndRot: "@Normalize(B.Rotation)@"	CurRot: "@Normalize(POV.Rotation)@"	Alpha: "@Alpha);
	}
	else
	{
		POV.Rotation	= RLerp(A.Rotation, B.Rotation, Alpha, TRUE);
	}

	return POV;
}

native function Rotator RInterpNormalRotationTo(const out Rotator RA, const vector VB, float Alpha);

// 重载CAMERA更新流程函数
function UpdateViewTarget(out TViewTarget OutVT, float DeltaTime)
{
	local CameraActor	CamActor;
	local bool			bDoNotApplyModifiers;

	// Don't update outgoing viewtarget during an interpolation
	if( PendingViewTarget.Target != None && OutVT == ViewTarget && BlendParams.bLockOutgoing )
	{
		return;
	}

	// Default FOV on viewtarget
	OutVT.POV.FOV = DefaultFOV;

	// Viewing through a camera actor.
	CamActor = CameraActor(OutVT.Target);
	if( CamActor != None )
	{
		CamActor.GetCameraView(DeltaTime, OutVT.POV);

		// Grab aspect ratio from the CameraActor.
		bConstrainAspectRatio	= bConstrainAspectRatio || CamActor.bConstrainAspectRatio;
		OutVT.AspectRatio		= CamActor.AspectRatio;

		// See if the CameraActor wants to override the PostProcess settings used.
		CamOverridePostProcessAlpha = CamActor.CamOverridePostProcessAlpha;
		CamPostProcessSettings = CamActor.CamOverridePostProcess;
	}
	else
	{
		// don't apply modifiers when using these debug camera modes.
		bDoNotApplyModifiers = FALSE;

		UpdateVT(OutVT, DeltaTime);

		UpdateCameraLensEffects(OutVT);
	}

	if( !bDoNotApplyModifiers )
	{
		// Apply camera modifiers at the end (view shakes for example)
		ApplyCameraModifiers(DeltaTime, OutVT.POV);
	}
}

function UpdateVT(out TViewTarget OutVT, float DeltaTime)
{
	_MouseDelta = SeaPlayerController(PCOwner)._MouseDelta;							// 获取鼠标当前帧的偏移量
	_MouseDeltaByDrag = SeaPlayerController(PCOwner)._MouseDeltaByDrag;				// 获取鼠标当前帧的偏移量
}

function BeginState(name PreviousStateName)
{
	local vector StartPos;
	local vector HitLoc;
	local SeaPlayerControllerFight PCFight;
    local LogicEventParam Params;

	SeaPlayerController(PCOwner).OnLogicEvent('CameraModeChange', Params);

	PCFight = SeaPlayerControllerFight(PCOwner);
	if( PCFight == none )
		return;
	if (PCOwner.Pawn == none)
	{
		`UCLog(, "SeaCamera BeginState PCOwner.Pawn == none");
		return;
	}

	if( _MaxSceneHeight == 0.f )
	{
		StartPos = PCOwner.Pawn != none ? PCOwner.Pawn.Location : vect(0,0,0);
		_MaxSceneHeight = CalcSceneHeight(StartPos) - 100.f;
	}

	if( _OceanZ == 0.f )
	{
		if( PCFight.OceanTrace(PCOwner.Pawn.Location, vect(0,0,-1), HitLoc) )
		{
			_OceanZ = HitLoc.Z;
		}
	}
}

function EndState(Name NextStateName)
{
	local SeaPlayerControllerFight PC;
	local vector StartPos, EndPos, HitLoc;

	PC = SeaPlayerControllerFight(PCOwner);
	if( PC != none )
	{
		StartPos = CameraCache.POV.Location;
		EndPos = StartPos + Normal(Vector(CameraCache.POV.Rotation)) * MAX_TRACE_RADIUS;
		if( PC.CameraTraceEx(StartPos, EndPos, 0.f, HitLoc) )
		{
			_PointLookAt = HitLoc;
			_bAlwaysLookAt = true;			// 可能满足条件始终LookAt，还需要进入NextStateName时进一步判断
		}
		else
		{
			_bAlwaysLookAt = false;
		}
	}
}

// 不同状态下修正CAMERA位置的偏移
function ModifyCameraLoc(out vector InCameraLoc, float Height);

/**
 * 鹰眼视角下移动CAMERA至Target
 */
function SetViewTargetPos(Vector2D InLookAt);

/**
 * 以目标点驱动camera
 */
function SetViewTargetPos3D(vector InLookAt);

/** 视距调远/调近
 * In/Out事件由PC驱动
*/
function OnZoomIn(int iSpeed);
function OnZoomOut(int iSpeed);

/** SHIFT响应函数
 * @param ActivePartType		当前激活的武器
 * @param ViewModeList			当前武器支持的视角类型
*/
function ToggleView(int ActivePartType, int ViewModeList);

function SwitchPartType(SwitchPartFeedback Feedback);

function SetViewLockParam(SeaPawn LockedPawn, int LockedDeltaYaw)
{
	_LockedPawn = LockedPawn;
	_LockedDeltaYaw = LockedDeltaYaw;
}

/** 当前相机目标是否是自己
 */
function bool IsWatchingSelf()
{
	return (ViewTarget.Target == PCOwner.Pawn);
}

/** 切换观察者
 * @param NewViewTarget 目标观察者
 * @param TransitionParams 切换观察者时镜头表现的设置,如淡入淡出效果
 */
function SwitchViewTarget(Actor NewViewTarget, optional ViewTargetTransitionParams TransitionParams)
{
	local ViewTargetTransitionParams NullParams;

	// 切换观察者时强制到第三人称
	SetViewTarget(NewViewTarget, NullParams);		// 仅切换ViewTarget

	SetCamMode('ThirdPersonView', true, TransitionParams, true);
}

function ViewTargetTransitionParams GetTransitionParamsByCameraStyle(name InCameraStyle)
{
	local ViewTargetTransitionParams Params;

	switch(InCameraStyle)
	{
	case 'FirstPersonView':
		Params = _FirstViewParam._BlendParams;
		break;
	case 'ThirdPersonView':
		Params = _FreeViewParam._BlendParams;
		break;
	case 'TelescopeView':
		Params = _TelescopeViewParam._BlendParams;
		break;
	case 'ArtilleryView':
		Params = _ArtilleryViewParam._BlendParams;
		break;
	case 'FightJetView':
		Params = _FightJetViewParam._BlendParams;
		break;
	case 'BirdView':
		Params = _BirdViewParam._BlendParams;
		break;
	case '_ObserverViewParam':
		Params = _ObserverViewParam._BlendParams;
		break;
	}
	return Params;
}

// 视角模式枚举值与策划配置的视角值转换接口
public function int TranslateViewFlag(name InCameraStyle)
{
	Local int ConfigFlag;

	switch(InCameraStyle)
	{
	case 'FirstPersonView':
		    ConfigFlag = FirstPerson;
	    break;
	case 'ThirdPersonView':
			ConfigFlag = ThirdPerson;
		break;
	case 'TelescopeView':
			ConfigFlag = Telescope;
		break;
	case 'ArtilleryView':
			ConfigFlag = ArtilleryEye;
		break;
	case 'FightJetView':
			ConfigFlag = FightJetEye;
		break;
	case 'BirdView':
			ConfigFlag = BirdEye;
		break;
    case 'FreeCameraView':
            ConfigFlag = FreeCamera;
        break;
	}

	return ConfigFlag;
}

// 策划配置的视角模式到camera style转换
private function name TranslateCameraStyle(int ViewFlag)
{
	local name CamMode;

	switch(ViewFlag)
	{
	case FreeCamera:
		CamMode = 'FreeCameraView';
		break;
	case FightJetEye:
		CamMode = 'FightJetView';
		break;
	case ArtilleryEye:
		CamMode = 'ArtilleryView';
		break;
	case Telescope:
		CamMode = 'TelescopeView';
		break;
	case ThirdPerson:
		CamMode = 'ThirdPersonView';
		break;
	case FirstPerson:
		CamMode = 'FirstPersonView';
		break;
	case BirdEye:
		CamMode = 'BirdView';
		break;
	default:
		CamMode = 'ThirdPersonView';
		break;
	}

	return CamMode;
}

/** 设置相机模式的唯一接口，考虑当前武器选择而限制的视角
 *  @param NewCameraMode
 *  @param bForceSwitch		是否考虑因武器的限制而强制切换视角
 *  @param bSwitchStateEvenTheSameStyle		TRUE，即使camerastyle不变仍切换style；false，反之
 *											当style不变，但ViewTarget发生变化时需要切换
 *	@param bUseSpecifiedLookAtPoint			切换至NextState时使用指定点为LookAtPoint
 *	@param SpecifiedLookAtPoint				指定点LookAtPoint，bUseSpecifiedLookAtPoint==true才有意义
 */
function bool SetCamMode(name NewCameraMode, 
						 optional bool bForceSwitch = false, 
						 optional ViewTargetTransitionParams TransitionParams, 
						 optional bool bSwitchStateEvenTheSameStyle = false,
						 optional bool bUseSpecifiedLookAtPoint = false,
						 optional vector SpecifiedLookAtPoint)
{
	local int ViewmodeList;
	local int PrioMode;
	local AnyParam Param;

	// 自由视角和大厅第三人称视角优先处理
	if (NewCameraMode == 'FreeCameraView' || NewCameraMode == 'ThirdPersonViewEx')
    {
		GotoState(NewCameraMode);
        return true;
    }

	if( !bForceSwitch )
	{ // 考虑武器的限制
		ViewModeList = SeaPlayerControllerFight(PCOwner).GetViewModeListByActiveType();
		if(ViewmodeList == 0)
		{
			`UClog(FUNC_TAG_None,"None viewmode list supported.");
			return false;
		}

		// 若当前选择的武器类型不支持此视角模式（EMode），则根据优先级选择一个最适合的视角模式
		if( (ViewmodeList & TranslateViewFlag(NewCameraMode)) == 0)
		{
			foreach _PriorityCamSwitchMode(PrioMode)
			{
				if((ViewmodeList & PrioMode) != 0)
				{
					NewCameraMode = TranslateCameraStyle(PrioMode);

					// 若输入的NewCameraMode发生改变，则根据NewCameraMode重新选择过渡参数
					TransitionParams = GetTransitionParamsByCameraStyle(NewCameraMode);
					break;
				}
			}
		}
	}

	if( CameraStyle != NewCameraMode || bSwitchStateEvenTheSameStyle )
	{
		// 通知服务端视角模式
		`SeaNetClient.ViewModeSelectPost(TranslateViewFlag(NewCameraMode));
	
		_bUseSpecifiedLookAtPoint = bUseSpecifiedLookAtPoint;
		_SpecifiedLookAtPoint = SpecifiedLookAtPoint;

		GotoState(NewCameraMode);

		_bUseSpecifiedLookAtPoint = false;		// 还原，使用默认模式切换视角

		BeginCameraSwitch(TransitionParams);

		// PVE新手教程事件
		Param.SS.AddItem(string(NewCameraMode));
		`ClientWorldInfo.InputPVEEvent(EPVETE_SwitchViewMode, Param);
	}

	return true;
}

// camera在不同模式之间切换的开始/结束
private function BeginCameraSwitch(ViewTargetTransitionParams TransitionParams)
{
	_BlendParamsWhenSwitch = TransitionParams;
	_BlendTimeWhenSwitch = TransitionParams.BlendTime;

	_BlendedStartPOV = CameraCache.POV;

	if( _bEnableBlend )
	{
		OnBeginCameraSwitch();
	}
	else
	{
		_BlendTimeWhenSwitch = 0.f;
	}
}

private function EndCameraSwitch()
{
	_BlendTimeWhenFollow = 0.f;			// 清除follow time，Switch阶段不处理follow请求
	_bAlwaysLookAt = false;
	OnEndCameraSwitch();
}

// camera在模式内跟随状态的开始/结束接口
private function BeginCameraFollow(ViewTargetTransitionParams TransitionParams,
								   optional bool bUseSpecifiedLookAtPoint = false,
								   optional vector SpecifiedLookAtPoint)
{
	_BlendParamsWhenFollow = TransitionParams;
	_BlendTimeWhenFollow = TransitionParams.BlendTime;
	_bUseSpecifiedLookAtPointWhenFollow = bUseSpecifiedLookAtPoint;
	_SpecifiedLookAtPointWhenFollow = SpecifiedLookAtPoint;

	_BlendedStartPOV = CameraCache.POV;

	if( _bEnableBlend )
	{
		OnBeginCameraFollow();
	}
	else
	{
		_BlendTimeWhenFollow = 0.f;
	}
}

private function EndCameraFollow()
{
	OnEndCameraFollow();
}

function OnBeginCameraSwitch();
function OnEndCameraSwitch();
function OnBeginCameraFollow();
function OnEndCameraFollow();

/** 从鹰眼视角切换为其他视角时,要设定一个合理的镜头朝向
 */
private function LookAtTarget()
{
	local Vector LookDir, Loc;
	local Rotator Rot;
	local vector LookAtPoint;

	ViewTarget.Target.GetActorEyesViewPoint(Loc, Rot);

	if( _bUseSpecifiedLookAtPoint )
	{
		LookAtPoint = _SpecifiedLookAtPoint;
	}
	else
	{
		LookAtPoint = SeaPlayerControllerFight(PCOwner)._LookAtPoint;
	}

	LookDir = Normal(LookAtPoint - Loc);
	Rot = Normalize(Rotator(LookDir));
	PCOwner.SetRotation(Rot);
}

// 调整视角，使用观察者朝向（忽略ROLL,PITCH）
private function UseViewTargetRotation(optional bool bIgnorePitch = true, optional bool bIgnoreRoll = true)
{
    local rotator ViewTargetRot;
	
    ViewTargetRot = ViewTarget.Target.Rotation;
	if( bIgnoreRoll )
		ViewTargetRot.Roll = 0;
	if( bIgnorePitch )
		ViewTargetRot.Pitch = 0;

	PCOwner.SetRotation(Normalize(ViewTargetRot));
}

function ModifyCameraRotation(float DeltaTime, out Rotator OutRot)
{
	local rotator BaseRot;
	local int FinalYaw;

	// 有锁定目标时
	if( _LockedPawn != none )
	{
		BaseRot = Rotator(_LockedPawn.Location - ViewTarget.Target.Location);
		FinalYaw = NormalizeRotAxis(BaseRot.Yaw + _LockedDeltaYaw + SeaPlayerControllerFight(PCOwner)._DeltaRot.Yaw);
		OutRot.Yaw = FinalYaw;
	}
}

state FirstPersonView
{
	function BeginState(name PreviousStateName)
	{
		local vector ViewLoc;
		local rotator ViewRot;
		local vector StartPos, EndPos, HitLoc;

		_PrevCameraStyle = PreviousStateName;
		CameraStyle = 'FirstPersonView';

        global.BeginState(PreviousStateName);

		if(PreviousStateName == 'FightJetView')
		{
            //UseViewTargetRotation();
            SeaPlayerControllerFight(PCOwner).RestoreCameraLookAt();
		}
        else
        {            
			LookAtTarget();
        }

		//if( PreviousStateName == 'ThirdPersonView' || PreviousStateName == 'TelescopeView')
		//	_bAlwaysLookAt = false;
		
		// 前一状态判断为_bAlwaysLookAt，才需要进一步判断
		if( _bAlwaysLookAt )
		{
			ViewTarget.Target.GetActorEyesViewPoint(ViewLoc, ViewRot);
			StartPos = ViewLoc;
			EndPos = StartPos + Normal(Vector(ViewRot)) * MAX_TRACE_RADIUS;
			_bAlwaysLookAt = false;
			if( SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0.f, HitLoc) )
			{
				// 前后两次判断容差在一定范围才认为相同
				if( Abs(HitLoc.X - _PointLookAt.X) < _ToleranceDistance && Abs(HitLoc.Y - _PointLookAt.Y) < _ToleranceDistance )
				{
					_bAlwaysLookAt = true;
					_PointLookAt = HitLoc;
				}
			}
		}
	}

	function EndState(Name NextStateName)
	{
		Super.EndState(NextStateName);
	}

	function ModifyCameraLoc(out vector InCameraLoc, float Height)
	{
		InCameraLoc.Z = _FirstViewParam._FirstPersonViewOffset * Height + _OceanZ;
	}

	function UpdateVT(out TViewTarget OutVT, float DeltaTime)
	{
		local vector TargetLoc;
		local rotator TargetRot;

		super.UpdateVT(OutVT, DeltaTime);

		ViewTarget.Target.GetActorEyesViewPoint(TargetLoc, TargetRot);

		ModifyCameraRotation(DeltaTime, TargetRot);

		OutVT.POV.Location = TargetLoc;
		OutVT.POV.Rotation = TargetRot;
		OutVT.POV.FOV = _FirstViewParam._CustomizedFirstFOV;

		PCOwner.SetRotation(OutVT.POV.Rotation);
	}
	
	function OnZoomOut(int iSpeed)
	{
		if(!PCOwner.IsInState('Dead'))
        {
			SetCamMode('TelescopeView',,_TelescopeViewParam._BlendParams);
        }
	}

	function OnZoomIn(int iSpeed)
	{
		SetCamMode('ThirdPersonView',,_FreeViewParam._BlendParams);
	}
	
	// first person
	function ToggleView(int ActivePartType, int ViewModeList)
	{
		// 当前武器若支持鹰眼视角则优先，否则狙击
		// 死亡状态无可能切换到第一人称，故不考虑
		if( ActivePartType == -1 && (ViewModeList & TranslateViewFlag('FightJetView')) != 0 )
		{ // 选中舰载机时
			SetCamMode('FightJetView',,_FightJetViewParam._BlendParams);
			return;
		}
	
		if( (ViewModeList & TranslateViewFlag('ArtilleryView')) != 0 )
		{
			SetCamMode('ArtilleryView',,_ArtilleryViewParam._BlendParams);
			return;
		}

		if( (ViewModeList & TranslateViewFlag('BirdView')) != 0 )
		{
			SetCamMode('BirdView',,_BirdViewParam._BlendParams);
			return;
		}		

		SetCamMode('TelescopeView',,_TelescopeViewParam._BlendParams);
	}
}

state TelescopeView extends FirstPersonView
{
	//add by yeleiyu@icee.cn for 狙击视角
	function StorePostProcessing()
	{
		if(OldPostProcessChain.length==0)
		{
			OldPostProcessChain = LocalPlayer(PCOwner.Player).PlayerPostProcessChains;
		}
	}

	function EndChangeView()
	{
		local int PPIdx;
		local LocalPlayer LP;
        local SeaPlayerControllerFight PC;
   		local SeaUIFightController UI;
		//add by yeleiyu@icee.cn for 狙击视角
		//狙击视角
		LP = LocalPlayer(PCOwner.Player);


		LP.RemoveAllPostProcessingChains();

		for(PPIdx=0; PPIdx<OldPostProcessChain.length; PPIdx++)
		{
			LP.InsertPostProcessingChain(OldPostProcessChain[PPIdx], -1, true);
		}
		OldPostProcessChain.length = 0;
	    PC = SeaPlayerControllerFight(PCOwner);
		UI = SeaUIFightController(`SeaUIController);
		PC.HidePeriscopeCross();
		if(UI!=none)
		{
			UI._CrossHair.ShowCrosshairIndicator();
		}
	}

	//add by yeleiyu@icee.cn for 狙击视角
	function DoChangeView()
	{
		local SeaPlayerControllerFight PC;
		local SeaUIFightController UI;
		local LocalPlayer LP;
		//add by yeleiyu@icee.cn for 狙击视角
		//狙击视角
		LP = LocalPlayer(PCOwner.Player);
		LP.RemoveAllPostProcessingChains();
		LP.InsertPostProcessingChain(TelescopeViewPPC, INDEX_NONE, false);

		TelescopeViewMaterialInstance.SetVectorParameterValue('little leaves', Resolution);

        PC = SeaPlayerControllerFight(PCOwner);
		UI = SeaUIFightController(`SeaUIController);
		if(  PC._PlayerData._FightingShip._SubmarineLayer < 0)
		{
			PC.ShowPeriscopeCross();
			UI._CrossHair.HideCrosshairIndicator();
		}
		else
		{
			PC.HidePeriscopeCross();
			UI._CrossHair.ShowCrosshairIndicator();
		}
	}

	function BeginState(name PreviousStateName)
	{
		local vector ViewLoc;
		local rotator ViewRot;
		local vector StartPos, EndPos, HitLoc;

		_PrevCameraStyle = PreviousStateName;
		CameraStyle = 'TelescopeView';

		global.BeginState(PreviousStateName);

		//add by yeleiyu@icee.cn for 狙击视角
		StorePostProcessing();
		DoChangeView();

		_TelescopeViewParam._CurScaleAlpha = 1.f;
		_TelescopeViewParam._CurrViewOffset = CameraCache.POV.Location.Z;
		_TelescopeViewParam._bUseCannon = false;
		if( SeaPlayerControllerFight(PCOwner)._PlayerData._FightingShip._ActivePartType == EPART_MainArtillery
			|| SeaPlayerControllerFight(PCOwner)._PlayerData._FightingShip._ActivePartType == EPART_SubArtillery )
		{
			_TelescopeViewParam._bUseCannon = true;
		}

		if( PreviousStateName == 'FirstPersonView' )
		{
			// 从第一人称切换来使用第一档
			SetZoomLevel(0);
		}
		else
		{
			// 进入狙击模式默认使用上次数据
		    SetZoomLevel(_TelescopeViewParam._CurrTelescopeLevel);
		}

        LookAtTarget();
		
		//if( PreviousStateName == 'ThirdPersonView' || PreviousStateName == 'FirstPersonView')
		//	_bAlwaysLookAt = false;
		
		// 前一状态判断为_bAlwaysLookAt，才需要进一步判断
		if( _bAlwaysLookAt )
		{
			ViewTarget.Target.GetActorEyesViewPoint(ViewLoc, ViewRot);
			StartPos = ViewLoc;
			EndPos = StartPos + Normal(Vector(ViewRot)) * MAX_TRACE_RADIUS;
			_bAlwaysLookAt = false;
			if( SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0.f, HitLoc) )
			{
				// 前后两次判断容差在一定范围才认为相同
				if( Abs(HitLoc.X - _PointLookAt.X) < _ToleranceDistance && Abs(HitLoc.Y - _PointLookAt.Y) < _ToleranceDistance )
				{
					_bAlwaysLookAt = true;
					_PointLookAt = HitLoc;
				}
			}
		}

		// PVP新手教程事件
		`ClientWorldInfo.InputPVPEvent(EPVPTE_SnipeView);
	}

	//add by yeleiyu@icee.cn for 狙击视角
	function EndState(Name NextStateName)
	{
		EndChangeView();

		Super.EndState(NextStateName);
	}

	function ModifyCameraLoc(out vector InCameraLoc, float Height)
	{
		local Vector ListenerLocScale;
		local Vector ListenerLocOffset;
		local int CurrLevel;

		CurrLevel = Clamp(_TelescopeViewParam._CurrTelescopeLevel, 0, _TelescopeViewParam._bUseCannon ? (_TelescopeViewParam._TelescopeViewOffset.Length - 1) : (_TelescopeViewParam._TelescopeViewOffsetExceptCannon.Length - 1));
		InCameraLoc.Z = _FirstViewParam._FirstPersonViewOffset * Height + (_TelescopeViewParam._bUseCannon ? _TelescopeViewParam._TelescopeViewOffset[CurrLevel] : _TelescopeViewParam._TelescopeViewOffsetExceptCannon[CurrLevel]) + _OceanZ;
		
		// 修正拾音器的Z值
		ListenerLocScale.X = 1.0;
		ListenerLocScale.Y = 1.0;
		ListenerLocScale.Z = 1.0;
		ListenerLocOffset.X = 0.0;
		ListenerLocOffset.Y = 0.0;
		ListenerLocOffset.Z = 200 - InCameraLoc.Z;
		LocalPlayer(PCOwner.Player).SetListenerLocFactor(ListenerLocScale, ListenerLocOffset);
	}

	function TPOV BlendViewTargetOfFollow(const out TPOV A,const out TPOV B, float Alpha)
	{
		local TPOV	POV;
		local float Length, Width, Height;

		_TelescopeViewParam._CurScaleAlpha = Alpha;

		SeaPlayerControllerFight(SeaPawn(ViewTarget.Target).Controller).GetPawnVolume(Length, Width, Height);

		POV.Location	= ViewTarget.Target.Location;
		POV.Location.Z = Lerp(_TelescopeViewParam._CurrViewOffset,
						   _FirstViewParam._FirstPersonViewOffset * Height + (_TelescopeViewParam._bUseCannon ? _TelescopeViewParam._TelescopeViewOffset[_TelescopeViewParam._CurrTelescopeLevel] : _TelescopeViewParam._TelescopeViewOffsetExceptCannon[_TelescopeViewParam._CurrTelescopeLevel]) + _OceanZ,
						   _TelescopeViewParam._CurScaleAlpha);
		

		POV.FOV			= Lerp(A.FOV, B.FOV, Alpha);
		if( _bUseSpecifiedLookAtPointWhenFollow )
		{
			POV.Rotation	= Rotator(_SpecifiedLookAtPointWhenFollow - POV.Location);
		}
		else
		{
			POV.Rotation	= RLerp(A.Rotation, B.Rotation, Alpha, TRUE);
		}

		PCOwner.SetRotation(POV.Rotation);

		return POV;
	}

	//add by yeleiyu@icee.cn for 狙击视角
	function UpdateVT(out TViewTarget OutVT, float DeltaTime)
	{
		local SeaPlayerControllerFight PC;
	    local SeaGameHUD hud;

		super.UpdateVT(OutVT, DeltaTime);

		OutVT.POV.FOV = _TelescopeViewParam._TelescopeZoomLevel[_TelescopeViewParam._CurrTelescopeLevel];
		
		GetCurResolution();
		if( TelescopeViewMaterialInstance != none && IsResolutionChange)
		{
			TelescopeViewMaterialInstance.SetVectorParameterValue('little leaves', Resolution);
		    PC = SeaPlayerControllerFight(`SeaGameEngine.GetPC());
			if(PC!=none && hud != none)
			{
		    	hud = SeaGameHUD(PC.MyHud);
				if(PC!=none && hud != none)
				{
					hud.SetMaterialResolution(Resolution);
				}
			}
			IsResolutionChange =False;
		}
	}

	/** 设置望远镜缩放等级
	 *  @param iLevel 设置FOV等级
	 *  @return 返回设置后FOV等级
	 */
	function int SetZoomLevel(int iLevel)
	{
		local int OldLevel;
		local vector SpecifiedLookAtPoint;

		OldLevel = _TelescopeViewParam._CurrTelescopeLevel;

		_TelescopeViewParam._CurrTelescopeLevel = iLevel;
		_TelescopeViewParam._CurrTelescopeLevel = Clamp(_TelescopeViewParam._CurrTelescopeLevel, 0, _TelescopeViewParam._TelescopeZoomLevel.Length-1);

		if( OldLevel != _TelescopeViewParam._CurrTelescopeLevel )
		{
			_TelescopeViewParam._CurScaleAlpha = 0.f;
			_TelescopeViewParam._CurrViewOffset = CameraCache.POV.Location.Z;

			SpecifiedLookAtPoint = SeaPlayerControllerFight(PCOwner)._LookAtPoint;
			BeginCameraFollow(_TelescopeViewParam._InternalBlendParams, true, SpecifiedLookAtPoint);
		}

		// 当camera仍处于Switch阶段或无follow time，立即更新到_PendingScaleAlpha
		if( _BlendTimeWhenFollow == 0.f )
		{
			_TelescopeViewParam._CurScaleAlpha = 1.f;
		}

		return _TelescopeViewParam._CurrTelescopeLevel;
	}

	function OnZoomOut(int iSpeed)
	{
		if( _bEnableBlend && _BlendTimeWhenSwitch > 0.f )
			return;

		SetZoomLevel(_TelescopeViewParam._CurrTelescopeLevel+1);
	}

	function OnZoomIn(int iSpeed)
	{
		if( _bEnableBlend && _BlendTimeWhenSwitch > 0.f )
			return;

		if(_TelescopeViewParam._CurrTelescopeLevel == 0 )
		{ // 当前已处于第一级FOV
			SetCamMode('FirstPersonView',,_FirstViewParam._BlendParams);
		}
		else
		{
			SetZoomLevel(_TelescopeViewParam._CurrTelescopeLevel-1);
		}
	}

	// 切换武器
	function SwitchPartType(SwitchPartFeedback Feedback)
	{
		local vector SpecifiedLookAtPoint;

		if( Feedback.bNextRegularWeapon )
		{
			if( SeaPlayerControllerFight(PCOwner)._PlayerData._FightingShip._ActivePartType == EPART_MainArtillery
				|| SeaPlayerControllerFight(PCOwner)._PlayerData._FightingShip._ActivePartType == EPART_SubArtillery )
			{
				_TelescopeViewParam._bUseCannon = true;
			}
			else
			{
				_TelescopeViewParam._bUseCannon = false;
			}
		}
		else
		{
			_TelescopeViewParam._bUseCannon = false;
		}

		_TelescopeViewParam._CurScaleAlpha = 0.f;
		_TelescopeViewParam._CurrViewOffset = CameraCache.POV.Location.Z;

		SpecifiedLookAtPoint = SeaPlayerControllerFight(PCOwner)._LookAtPoint;
		BeginCameraFollow(_TelescopeViewParam._InternalBlendParams, true, SpecifiedLookAtPoint);
	}

	// telescop view
	function ToggleView(int ActivePartType, int ViewModeList)
	{
		local name PrevCamera;

		// 切换至之前的视角模式
		if( ActivePartType != -1 )
		{
			// 支持鹰眼模式则优先切换
			if( (ViewModeList & TranslateViewFlag('ArtilleryView')) != 0 )
			{
				PrevCamera = _PrevCameraStyle;
				SetCamMode('ArtilleryView',,_ArtilleryViewParam._BlendParams);
				_PrevCameraStyle = PrevCamera;
			}
			else if( (ViewModeList & TranslateViewFlag('BirdView')) != 0 )
			{
				PrevCamera = _PrevCameraStyle;
				SetCamMode('BirdView',,_BirdViewParam._BlendParams);
				_PrevCameraStyle = PrevCamera;
			}
			else
			{
				SetCamMode('ThirdPersonView',,_FreeViewParam._BlendParams);
			}
		}
		else
		{
			SetCamMode('FightJetView',,_FightJetViewParam._BlendParams);
		}
	}
}

state ThirdPersonView
{
	function BeginState(name PreviousStateName)
	{
		local vector ViewLoc;
		local rotator ViewRot;
		local vector StartPos, EndPos, HitLoc;
		
		_PrevCameraStyle = PreviousStateName;
		CameraStyle = 'ThirdPersonView';

		global.BeginState(PreviousStateName);



		InitViewParam();
		
		if( PCOwner.IsInState('Dead') )
		{ // 死亡视角切换时强制使用不盯着LookAtPoint点算法
			_bAlwaysLookAt = false;
		}
		else
		{
			if(PreviousStateName == 'FightJetView')
			{
                //UseViewTargetRotation();
                SeaPlayerControllerFight(PCOwner).RestoreCameraLookAt();
			}
			else 
			{
				LookAtTarget();
			}
		}

		//if( PreviousStateName == 'TelescopeView' || PreviousStateName == 'FirstPersonView')
		//	_bAlwaysLookAt = false;
		
		// 前一状态判断为_bAlwaysLookAt，才需要进一步判断
		if( _bAlwaysLookAt )
		{
			ViewTarget.Target.GetActorEyesViewPoint(ViewLoc, ViewRot);
			StartPos = ViewLoc;
			EndPos = StartPos + Normal(Vector(ViewRot)) * MAX_TRACE_RADIUS;
			_bAlwaysLookAt = false;
			if(SeaPlayerControllerFight(PCOwner) != none && SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0.f, HitLoc) )
			{
				// 前后两次判断容差在一定范围才认为相同
				if( Abs(HitLoc.X - _PointLookAt.X) < _ToleranceDistance && Abs(HitLoc.Y - _PointLookAt.Y) < _ToleranceDistance )
				{
					_bAlwaysLookAt = true;
					_PointLookAt = HitLoc;
				}
			}
		}
	}

	function EndState(name NextStateName)
	{
		Super.EndState(NextStateName);
	}

	function InitViewParam()
	{		
		local float Length, Width, Height;
		local int Index;
		local float Scale;

		_FreeViewParam._LimitPitchMax = Abs(_FreeViewParam._LimitPitchMax);

		// 每档基于基础长半轴的比值
		_FreeViewParam._ScaleLevelList.Length = 0;
		foreach _FreeViewParam._ScaleExpandLongAxle(Scale, Index)
		{
			_FreeViewParam._ScaleLevelList[Index] = Scale / _FreeViewParam._ScaleExpandLongAxle[_FreeViewParam._ScaleExpandLongAxle.Length - 1];
		}
				
		if( _PrevCameraStyle == 'BirdView' )
		{ // 至鸟瞰模式切换来默认进入第二档
			_FreeViewParam._CurAxleLevel = 1;
		}
		else
		{
			_FreeViewParam._CurAxleLevel = Clamp(_FreeViewParam._CurAxleLevel, 0, _FreeViewParam._ScaleExpandLongAxle.Length - 1);
		}
		_FreeViewParam._StartScaleAlpha = _FreeViewParam._ScaleLevelList[_FreeViewParam._CurAxleLevel];
		_FreeViewParam._CurScaleAlpha = _FreeViewParam._StartScaleAlpha;
		_FreeViewParam._PendingScaleAlpha = _FreeViewParam._StartScaleAlpha;
		_FreeViewParam._ScaleAlpha = 0.f;

		// 基础长短半轴长
		SeaPlayerControllerFight(PCOwner).GetPawnVolume(Length, Width, Height);
		if(_FreeViewParam._FixLength != 0)				// _FixLength不为0，则取代舰船配置的Length
			Length = _FreeViewParam._FixLength;
		_FreeViewParam._BasicHalfLongAxle = 0.5f * Length * _FreeViewParam._ScaleExpandLongAxle[_FreeViewParam._ScaleExpandLongAxle.Length - 1];
		_FreeViewParam._BasicHalfShortAxle = _FreeViewParam._BasicHalfLongAxle * _FreeViewParam._Ratio;
	}

	function ModifyCameraLoc(out vector InCameraLoc, float Height)
	{
		local float ViewOffset;

		ViewOffset = InCameraLoc.Z - _OceanZ;
		InCameraLoc.Z = ViewOffset * _FreeViewParam._ViewPosHeightRatio + _OceanZ;
	}

	function TPOV BlendViewTargetOfFollow(const out TPOV A,const out TPOV B, float Alpha)
	{
		_FreeViewParam._ScaleAlpha = Alpha;

		return ViewTarget.POV;
	}

	function UpdateVT(out TViewTarget OutVT, float DeltaTime)
	{
		local vector ViewLoc, Pos, HitLocation;
		local rotator ViewRot;
		local int LimitPitch;
				
		super.UpdateVT(OutVT, DeltaTime);

		// ViewLoc是视角LOOKAT点
		OutVT.Target.GetActorEyesViewPoint(ViewLoc, ViewRot);

		// 1. update fov
		OutVT.POV.FOV = _FreeViewParam._DefaultFOV;

		// 2. update rotation
		ModifyCameraRotation(DeltaTime, ViewRot);
		OutVT.POV.Rotation = ViewRot;

		// 3. update location
		LimitPitch = OutVT.POV.Rotation.Pitch;
		if( NormalizeRotAxis(LimitPitch) > _FreeViewParam._LimitPitchMax * DegToUnrRot )
		{
			LimitPitch = _FreeViewParam._LimitPitchMax * DegToUnrRot;
		}		

		_FreeViewParam._CurScaleAlpha = Lerp(_FreeViewParam._StartScaleAlpha, _FreeViewParam._PendingScaleAlpha, _FreeViewParam._ScaleAlpha);
		Pos = UpdateFreeViewParam(_FreeViewParam._BasicHalfLongAxle * _FreeViewParam._CurScaleAlpha, _FreeViewParam._BasicHalfShortAxle * _FreeViewParam._CurScaleAlpha,
									SeaPawn(OutVT.Target).GetRotationNoSink(), OutVT.POV.Rotation, LimitPitch, DeltaTime);
		Pos = Pos >> SeaPawn(OutVT.Target).GetRotationNoSink();
		Pos += ViewLoc;

		if( SeaPlayerControllerFight(PCOwner).CameraTraceEx(ViewLoc, Pos, 40, HitLocation, true) )
		{
			OutVT.POV.Location = HitLocation;
		}
		else
		{
			OutVT.POV.Location = Pos;
		}
		PCOwner.SetRotation(OutVT.POV.Rotation);

		
	}

	// 计算camera在以LOOKAT点为中心的椭圆空间的位置
	// @BasicHalfLongAxle		椭圆空间长半轴
	// @BasicHalfShortAxle		椭圆空间短半轴
	// @TargetRot				舰船朝向
	// @CameraRot				Camera朝向
	function vector UpdateFreeViewParam(float BasicHalfLongAxle, float BasicHalfShortAxle,
										  rotator TargetRot, rotator CameraRot, int LimitPitch, float DeltaTime)
	{
		local matrix RotTM;
		local vector LocalDir;	// 舰船坐标空间的视角朝向
		local vector LocalView;
		local float CosYaw;
		local float PitchRad; 
		local rotator Rot, TargetRotation;
		local float TanPitch;
		local float Temp, TanYaw;
		local float CurHalfLongAxle, CurHalfShortAxle;

		PitchRad = FClamp(NormalizeRotAxis(LimitPitch), -16200, 16200) * UnrRotToRad;
		TanPitch = Tan(PitchRad);

		Temp = Sqrt(Square(BasicHalfShortAxle) + Square(BasicHalfLongAxle * TanPitch));
		CurHalfLongAxle = BasicHalfLongAxle * BasicHalfShortAxle / Temp;
		CurHalfShortAxle = BasicHalfShortAxle * CurHalfLongAxle / BasicHalfLongAxle;

		// 视角在Target坐标空间的朝向
		Rot = CameraRot;
		Rot.Pitch = 0;
		Rot.Roll = 0;
        TargetRotation = TargetRot;
        TargetRotation.Pitch = 0;
        TargetRotation.Roll = 0;
		RotTM = MakeRotationMatrix(TargetRotation);		// 舰船坐标空间
		LocalDir = InverseTransformNormal(RotTM, Normal(Vector(Rot)));

		CosYaw = (LocalDir * -1.f) Dot vect(1,0,0);

		TanYaw = Tan(Acos(CosYaw));
		Temp = Sqrt(Square(CurHalfShortAxle) + Square(CurHalfLongAxle * TanYaw));
		LocalView.X = CurHalfShortAxle * CurHalfLongAxle / Temp * (CosYaw > 0.f ? 1.f : -1.f);
		LocalView.Y = LocalView.X * TanYaw * (((LocalDir * -1) Dot vect(0,1,0)) >= 0.f ? 1.f : -1.f);

		LocalView.Z = CurHalfLongAxle * TanPitch * -1.f;

		return LocalView;
	}
	
	function OnZoomOut(int iSpeed)
	{
		local int OldLevel;

		if(_FreeViewParam._CurAxleLevel == 0)
		{
			SetCamMode('FirstPersonView',,_FirstViewParam._BlendParams);
		}
		else
		{
			OldLevel = _FreeViewParam._CurAxleLevel;
			_FreeViewParam._CurAxleLevel = Clamp(--_FreeViewParam._CurAxleLevel, 0, _FreeViewParam._ScaleExpandLongAxle.Length-1);

			if (OldLevel != _FreeViewParam._CurAxleLevel)
			{
				_FreeViewParam._StartScaleAlpha = _FreeViewParam._CurScaleAlpha;	// 当前scale alpha作为起始scale alpha
				_FreeViewParam._PendingScaleAlpha = _FreeViewParam._ScaleLevelList[_FreeViewParam._CurAxleLevel];
				_FreeViewParam._ScaleAlpha = 0.f;

				BeginCameraFollow(_FreeViewParam._InternalBlendParams);
			}

			// 当camera仍处于Switch阶段或无follow time，立即更新到_PendingScaleAlpha
			if( _BlendTimeWhenSwitch > 0.f || _BlendTimeWhenFollow == 0.f )
			{
				_FreeViewParam._ScaleAlpha = 1.f;
			}
		}
	}

	function OnZoomIn(int iSpeed)
	{
		local int OldLevel;

		OldLevel = _FreeViewParam._CurAxleLevel;		
		_FreeViewParam._CurAxleLevel = clamp(++_FreeViewParam._CurAxleLevel, 0, _FreeViewParam._ScaleExpandLongAxle.Length-1);

		if( OldLevel != _FreeViewParam._CurAxleLevel )
		{
			_FreeViewParam._StartScaleAlpha = _FreeViewParam._CurScaleAlpha;	// 当前scale alpha作为起始scale alpha
			_FreeViewParam._PendingScaleAlpha = _FreeViewParam._ScaleLevelList[_FreeViewParam._CurAxleLevel];
			_FreeViewParam._ScaleAlpha = 0.f;

			BeginCameraFollow(_FreeViewParam._InternalBlendParams);
		}

		// 当camera仍处于Switch阶段或无follow time，立即更新到_PendingScaleAlpha
		if( _BlendTimeWhenSwitch > 0.f || _BlendTimeWhenFollow == 0.f )
		{
			_FreeViewParam._ScaleAlpha = 1.f;
		}
	}
	
	function OnEndCameraFollow()
	{
		_FreeViewParam._CurScaleAlpha = _FreeViewParam._PendingScaleAlpha;
		_FreeViewParam._ScaleAlpha = 0.f;
	}

	// third view
	function ToggleView(int ActivePartType, int ViewModeList)
	{
		// 当前武器若支持鹰眼视角则优先，否则狙击
		if(!PCOwner.IsInState('Dead'))
		{
			if( (ViewModeList & TranslateViewFlag('ArtilleryView')) != 0
				|| (ViewModeList & TranslateViewFlag('FightJetView')) != 0)
			{
				if( ActivePartType == -1 )
				{ // 选中舰载机时
					SetCamMode('FightJetView',,_FightJetViewParam._BlendParams);
				}
				else
				{
					SetCamMode('ArtilleryView',,_ArtilleryViewParam._BlendParams);
				}
				return;
			}

			if( (ViewModeList & TranslateViewFlag('BirdView')) != 0 )
			{
				SetCamMode('BirdView',,_BirdViewParam._BlendParams);
				return;
			}

			SetCamMode('TelescopeView',,_TelescopeViewParam._BlendParams);
		}
		else
		{
			// 死亡状态下在鹰眼模式切换，而不考虑武器类型是否支持
			SetCamMode('ArtilleryView', true, _ArtilleryViewParam._BlendParams);
		}
	}
}

// 大厅展示舰船时的第三人称视角
state ThirdPersonViewEx extends ThirdPersonView
{
	function BeginState(name PreviousStateName)
	{
		_PrevCameraStyle = PreviousStateName;
		CameraStyle = 'ThirdPersonViewEx';

        global.BeginState(PreviousStateName);

		InitViewParam();
	}

	function InitViewParam()
	{		
		local float Length, Width, Height;
		local int Index;
		local float Scale;

		_FreeViewParamEx._LimitPitchMax = Abs(_FreeViewParamEx._LimitPitchMax);

		// 每档基于基础长半轴的比值
		_FreeViewParamEx._ScaleLevelList.Length = 0;
		foreach _FreeViewParamEx._ScaleExpandLongAxle(Scale, Index)
		{
			_FreeViewParamEx._ScaleLevelList[Index] = Scale / _FreeViewParamEx._ScaleExpandLongAxle[_FreeViewParamEx._ScaleExpandLongAxle.Length - 1];
		}

		_FreeViewParamEx._CurAxleLevel = Clamp(_FreeViewParamEx._CurAxleLevel, 0, _FreeViewParamEx._ScaleExpandLongAxle.Length - 1);

		_FreeViewParamEx._StartScaleAlpha = _FreeViewParamEx._ScaleLevelList[_FreeViewParam._CurAxleLevel];
		_FreeViewParamEx._CurScaleAlpha = _FreeViewParamEx._StartScaleAlpha;
		_FreeViewParamEx._PendingScaleAlpha = _FreeViewParamEx._StartScaleAlpha;
		_FreeViewParamEx._ScaleAlpha = 0.f;
		
		// 基础长短半轴长
		SeaPlayerControllerLobby(PCOwner).GetPawnVolume(Length, Width, Height);
		if(_FreeViewParamEx._FixLength != 0)				// _FixLength不为0，则取代舰船配置的Length
			Length = _FreeViewParamEx._FixLength;
		_FreeViewParamEx._BasicHalfLongAxle = 0.5f * Length * _FreeViewParamEx._ScaleExpandLongAxle[_FreeViewParamEx._ScaleExpandLongAxle.Length - 1];
		_FreeViewParamEx._BasicHalfShortAxle = _FreeViewParamEx._BasicHalfLongAxle * _FreeViewParamEx._Ratio;
	}

	function ModifyCameraLoc(out vector InCameraLoc, float Height)
	{
		local float ViewOffset;

		ViewOffset = InCameraLoc.Z - _OceanZ;
		InCameraLoc.Z = ViewOffset * _FreeViewParamEx._ViewPosHeightRatio + _OceanZ;
	}

	function TPOV BlendViewTargetOfFollow(const out TPOV A,const out TPOV B, float Alpha)
	{
		_FreeViewParamEx._ScaleAlpha = Alpha;

		return ViewTarget.POV;
	}

	function UpdateVT(out TViewTarget OutVT, float DeltaTime)
	{
		local vector ViewLoc, Pos;
		local rotator ViewRot;
		local int LimitPitch;
        local vector HitLocation, ViewDir;

		Global.UpdateVT(OutVT, DeltaTime);

		// ViewLoc是视角LOOKAT点
		OutVT.Target.GetActorEyesViewPoint(ViewLoc, ViewRot);
		OutVT.POV.Rotation = ViewRot;
		OutVT.POV.FOV = _FreeViewParamEx._DefaultFOV;
		
		LimitPitch = OutVT.POV.Rotation.Pitch;
		if( NormalizeRotAxis(LimitPitch) > _FreeViewParamEx._LimitPitchMax * DegToUnrRot )
		{
			LimitPitch = _FreeViewParamEx._LimitPitchMax * DegToUnrRot;
		}
		if( NormalizeRotAxis(LimitPitch) < _FreeViewParamEx._LimitPitchMin * DegToUnrRot )
		{
			LimitPitch = _FreeViewParamEx._LimitPitchMin * DegToUnrRot;
		}

		_FreeViewParamEx._CurScaleAlpha = Lerp(_FreeViewParamEx._StartScaleAlpha, _FreeViewParamEx._PendingScaleAlpha, _FreeViewParamEx._ScaleAlpha);
		Pos = UpdateFreeViewParam(_FreeViewParamEx._BasicHalfLongAxle * _FreeViewParamEx._CurScaleAlpha, _FreeViewParamEx._BasicHalfShortAxle * _FreeViewParamEx._CurScaleAlpha,
								  OutVT.Target.Rotation, OutVT.POV.Rotation, LimitPitch, DeltaTime);
		Pos = Pos >> OutVT.Target.Rotation;
		Pos += ViewLoc;
        ViewDir = Normal(ViewLoc - Pos);

		if (SeaPlayerControllerLobby(PCOwner).CameraTraceEx(Pos, ViewLoc, 40, HitLocation))
		{
            //  摄像机到观察点射线检测到碰撞盒（摄像机在盒内）
            if (VSize(HitLocation - Pos) < _FreeViewParamEx._RejectParam)
                OutVT.POV.Location = HitLocation - ViewDir * _FreeViewParamEx._RejectParam;
            else
			    OutVT.POV.Location = Pos;
		}
		else if (SeaPlayerControllerLobby(PCOwner).CameraTraceEx(Pos - ViewDir * 100000, Pos, 0, HitLocation))
		{
            //  从摄像机位置外，在视线反方向延长线上的某点向摄像机位置作射线检测（摄像机在盒内）
			OutVT.POV.Location = HitLocation - ViewDir * _FreeViewParamEx._RejectParam;
		}
        else
        {
			OutVT.POV.Location = Pos;
        }
	}

	function OnZoomOut(int iSpeed)
	{
		local int OldLevel;

		OldLevel = _FreeViewParamEx._CurAxleLevel;
		_FreeViewParamEx._CurAxleLevel = Clamp(--_FreeViewParamEx._CurAxleLevel, 0, _FreeViewParamEx._ScaleExpandLongAxle.Length-1);

		if (OldLevel != _FreeViewParamEx._CurAxleLevel)
		{
			_FreeViewParamEx._StartScaleAlpha = _FreeViewParamEx._CurScaleAlpha;	// 当前scale alpha作为起始scale alpha
			_FreeViewParamEx._PendingScaleAlpha = _FreeViewParamEx._ScaleLevelList[_FreeViewParamEx._CurAxleLevel];
			_FreeViewParamEx._ScaleAlpha = 0.f;

			BeginCameraFollow(_FreeViewParamEx._InternalBlendParams);
		}

		// 当camera仍处于Switch阶段或无follow time，立即更新到_PendingScaleAlpha
		if( _BlendTimeWhenSwitch > 0.f || _BlendTimeWhenFollow == 0.f )
		{
			_FreeViewParamEx._ScaleAlpha = 1.f;
		}
	}

	function OnZoomIn(int iSpeed)
	{
		local int OldLevel;

		OldLevel = _FreeViewParamEx._CurAxleLevel;		
		_FreeViewParamEx._CurAxleLevel = clamp(++_FreeViewParamEx._CurAxleLevel, 0, _FreeViewParamEx._ScaleExpandLongAxle.Length-1);

		if( OldLevel != _FreeViewParamEx._CurAxleLevel )
		{
			_FreeViewParamEx._StartScaleAlpha = _FreeViewParamEx._CurScaleAlpha;	// 当前scale alpha作为起始scale alpha
			_FreeViewParamEx._PendingScaleAlpha = _FreeViewParamEx._ScaleLevelList[_FreeViewParamEx._CurAxleLevel];
			_FreeViewParamEx._ScaleAlpha = 0.f;

			BeginCameraFollow(_FreeViewParamEx._InternalBlendParams);
		}

		// 当camera仍处于Switch阶段或无follow time，立即更新到_PendingScaleAlpha
		if( _BlendTimeWhenSwitch > 0.f || _BlendTimeWhenFollow == 0.f )
		{
			_FreeViewParamEx._ScaleAlpha = 1.f;
		}
	}
	
	function OnEndCameraFollow()
	{
		_FreeViewParamEx._CurScaleAlpha = _FreeViewParamEx._PendingScaleAlpha;
		_FreeViewParamEx._ScaleAlpha = 0.f;
	}
}

native function float CalcSceneHeight(vector Start);

state EagleEyeView
{
	function BeginState(name PreviousStateName)
	{
		local Vector ListenerLocScale;
		local Vector ListenerLocOffset;
		
		_PrevCameraStyle = PreviousStateName;
		
        global.BeginState(PreviousStateName);

		ClearCameraLensEffects();		// ???

		// 设置鹰眼视角灵敏度
		if (PCOwner.PlayerInput != none)
			PCOwner.PlayerInput.SetSensitivity(`CustomDataMgr._CtrlSettingsData._SpecialMouseSensitivity);

		// 设置拾音器偏移量
		ListenerLocScale.X = 1.0;
		ListenerLocScale.Y = 1.0;
		ListenerLocScale.Z = _ListenerHeightScale;
		ListenerLocOffset.X = 0.0;
		ListenerLocOffset.Y = 0.0;
		ListenerLocOffset.Z = _ListenerHeightOffset;
		LocalPlayer(PCOwner.Player).SetListenerLocFactor(ListenerLocScale, ListenerLocOffset);

		// 鹰眼视角背景音效
		SwitchJetViewSound(true);

		if( !_bEnableBlend )
		{ // 无过渡时
			// 显示小地图可视区域
			SeaUIFightController(`SeaUIController).ShowMiniMapViewArea(true);
		}
	}

	function EndState(name NextStateName)
	{
		local SeaUIFightController UIFight;

		// 恢复普通视角灵敏度
		if (PCOwner.PlayerInput != none)
			PCOwner.PlayerInput.SetSensitivity(`CustomDataMgr._CtrlSettingsData._NormalMouseSensitivity);

		// 恢复拾音器偏移量
		LocalPlayer(PCOwner.Player).ResetListenerLocFactor();

		UIFight = SeaUIFightController(`SeaUIController);
		if( UIFight != none )
		{
			// 隐藏小地图可视区域
			UIFight.ShowMiniMapViewArea(false);
		}

		SwitchJetViewSound(false);

		Super.EndState(NextStateName);
	}

	function OnEndCameraSwitch()
	{
		// 显示小地图可视区域
		SeaUIFightController(`SeaUIController).ShowMiniMapViewArea(true);
	}
}

// 把输入参数规整到目标区域
// 返回值表示输入参数规整前是否在目标区域内
private function bool ClampToBoundArea(out vector OutPos, vector InPos, vector2D MinLoc, vector2D MaxLoc, vector2D Extent)
{
	if( (InPos.X < MinLoc.X - Extent.X) || (InPos.X > MaxLoc.X + Extent.X) || (InPos.Y < MinLoc.Y - Extent.Y) || (InPos.Y > MaxLoc.Y + Extent.Y) )
	{
		OutPos.X = FClamp(InPos.X, MinLoc.X - Extent.X, MaxLoc.X + Extent.X);
		OutPos.Y = FClamp(InPos.Y, MinLoc.Y - Extent.Y, MaxLoc.Y + Extent.Y);
		return false;
	}
	return true;
}

state ArtilleryView extends EagleEyeView
{
	// 假设从其他视角下切换至鹰眼视角，必须有上一帧的camera参数
	// 暂不支持初始为鹰眼视角模式
	function BeginState(name PreviousStateName)
	{		
		local vector ValidLocation;
		local vector StartPos, EndPos, HitLoc;
		
		CameraStyle = 'ArtilleryView';

		Super.BeginState(PreviousStateName);

		// init view param
		if( _ArtilleryViewParam._CurrentZoomOut == 0 )
		{
			_ArtilleryViewParam._CurrentZoomOut = _ArtilleryViewParam._DefaultZoomOut;
		}

		// step 1. 获得有效位置数据
		ValidLocation = InitValidLocation();
		 
		// step 2. push data to ViewTarget
		InitViewTargetParam(ValidLocation);

		// 前一状态判断为_bAlwaysLookAt，才需要进一步判断
		if( _bAlwaysLookAt )
		{
			StartPos = ViewTarget.POV.Location;
			EndPos = StartPos + Normal(Vector(ViewTarget.POV.Rotation)) * MAX_TRACE_RADIUS;
			_bAlwaysLookAt = false;
			if( SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0.f, HitLoc) )
			{
				// 前后两次判断容差在一定范围才认为相同
				if( Abs(HitLoc.X - _PointLookAt.X) < _ToleranceDistance && Abs(HitLoc.Y - _PointLookAt.Y) < _ToleranceDistance )
				{
					_bAlwaysLookAt = true;
					_PointLookAt = HitLoc;
				}
			}
		}

		// PVP新手教程事件
		`ClientWorldInfo.InputPVPEvent(EPVPTE_EagleEyeView);
	}

	function EndState(name NextStateName)
	{
		Super.EndState(NextStateName);

        SeaPlayerControllerFight(PCOwner).UnlockTarget(false);
	}

	// 获得合法的起始目标点
	// @param StartPos	观察位置
	// @param Dir		观察朝向
	// @param MinBound/MaxBound/MaxHeight	最小边界值/最大边界值/最大高度
	function vector GetValidLocation(vector StartPos, vector Dir, vector2D MinBound, vector2D MaxBound, float MaxHeight)
	{
		local vector EndPos;
		local vector HitLoc;
		local bool bTraceAnything;

		EndPos = StartPos + Dir * MAX_TRACE_RADIUS;
		bTraceAnything = SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0, HitLoc, true);

		// 检测不到物体时使用起始点计算
		ClampToBoundArea(HitLoc, bTraceAnything ? HitLoc : StartPos, MinBound, MaxBound, vect2d(-50, -50));

		// HitLoc是从其他视角检测到的目标点，需再次检测
		StartPos = HitLoc;
		StartPos.Z = MaxHeight;
		EndPos = StartPos + vect(0,0,-1) * MAX_TRACE_RADIUS;
		if( !SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0, HitLoc, true) )
		{
			`UClog(FUNC_TAG_None,"GetValidLocation failed!");
			HitLoc.Z = 0;
			return HitLoc;
		}
		return HitLoc;
	}

	// 获得有效位置数据
	function vector InitValidLocation()
	{
		local vector CameraLoc;
		local vector CameraDir;
		local vector ValidLocation;			// 切换至鹰眼的初始合法位置
		
		CameraLoc = CameraCache.POV.Location;						// 上一帧Camera位置
		CameraDir = Normal(Vector(CameraCache.POV.Rotation));		// 上一帧Camera朝向
		ValidLocation = GetValidLocation(CameraLoc, CameraDir, _MinBound, _MaxBound, _MaxSceneHeight);

		return ValidLocation;
	}

	function InitViewTargetParam(vector ValidLoc)
	{
		ViewTarget.POV.Rotation = _ArtilleryViewParam._DefaultCamRotation;
		ViewTarget.POV.Location = ValidLoc;
		ViewTarget.POV.Location -= _ArtilleryViewParam._CurrentZoomOut * Normal(Vector(_ArtilleryViewParam._DefaultCamRotation));
		ViewTarget.POV.FOV = _ArtilleryViewParam._DefaultFOV;
	}
		
	// 切换至鹰眼视角
	// @param TargetPos		以TargetPos为基点
	function SetViewTargetPos(Vector2D InLookAt)
	{
		local vector StartPos, EndPos;
		local vector HitLoc;
		
		StartPos.X = InLookAt.X;
		StartPos.Y = InLookAt.Y;
		StartPos.Z = _MaxSceneHeight;
		EndPos = StartPos + vect(0,0,-1) * MAX_TRACE_RADIUS;
					
		// 把参数StartPos限制在区域内
		ClampToBoundArea(StartPos, StartPos, _MinBound, _MaxBound, vect2d(-100,-100));

		if( !SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0, HitLoc, true) )
		{
			HitLoc = StartPos;
			HitLoc.Z = 0;
		}

		InitViewTargetParam(HitLoc);

		BeginCameraFollow(_ArtilleryViewParam._InternalBlendParams);
	}

	function UpdateVT(out TViewTarget OutVT, float DeltaTime)
	{
		local vector Delta, HitLocation;
		local vector NewPos, OldPos;
		local bool bHit;
		local vector2D CameraDelta;

		super.UpdateVT(OutVT, DeltaTime);
		
		CameraDelta.X = _MouseDelta.Y * _ArtilleryViewParam._EagleEyeMoveSpeed;
		CameraDelta.Y = _MouseDelta.X * _ArtilleryViewParam._EagleEyeMoveSpeed;

		// step1. X方向偏移的碰撞检测
		if (CameraDelta.X != 0.0f)
		{
			NewPos = OutVT.POV.Location;
			NewPos.X += CameraDelta.X;
			OldPos = OutVT.POV.Location;

			bHit = SeaPlayerControllerFight(PCOwner).CameraTraceEx(OldPos, NewPos, 10, HitLocation, true);
			if( bHit )
				Delta.X = HitLocation.X - OldPos.X;
			else
				Delta.X = NewPos.X - OldPos.X;
		}

		// step2. Y方向偏移的碰撞检测
		if (CameraDelta.Y != 0.0f)
		{
			NewPos = OutVT.POV.Location;
			NewPos.Y += CameraDelta.Y;
			OldPos = OutVT.POV.Location;

			bHit = SeaPlayerControllerFight(PCOwner).CameraTraceEx(OldPos, NewPos, 10, HitLocation, true);
			if( bHit )
				Delta.Y = HitLocation.Y - OldPos.Y;
			else
				Delta.Y = NewPos.Y - OldPos.Y;
		}

		if (Delta.X != 0 || Delta.Y !=0)
		{
			OutVT.POV.Location.X += Delta.X;
			OutVT.POV.Location.Y += Delta.Y;
		}

		// step3. Z方向偏移的碰撞检测
		SeaPlayerControllerFight(PCOwner).CameraTraceEx(OutVT.POV.Location, OutVT.POV.Location + vect(0,0,-1)*MAX_TRACE_RADIUS, 0, HitLocation, true);

		OutVT.POV.Location.Z = HitLocation.Z + _ArtilleryViewParam._CurrentZoomOut;
        OutVT.POV.Location.Z = Min(OutVT.POV.Location.Z, _MaxSceneHeight);
	}
	
	function OnZoomIn(int iSpeed)
	{
		local float OldZoomOut;

		OldZoomOut = _ArtilleryViewParam._CurrentZoomOut;

		_ArtilleryViewParam._CurrentZoomOut += Abs(_ArtilleryViewParam._EagleEyeZoomSpeed);
		_ArtilleryViewParam._CurrentZoomOut = FClamp(_ArtilleryViewParam._CurrentZoomOut, _ArtilleryViewParam._MinZoomOut, _ArtilleryViewParam._MaxZoomOut);

		// SetViewTargetPos驱动的FOLLOW优先级更高，此时不设置ZOOM IN/ZOOM OUT的过渡参数
		if( _BlendTimeWhenFollow == 0.f && OldZoomOut != _ArtilleryViewParam._CurrentZoomOut )
		{
			BeginCameraFollow(_ArtilleryViewParam._InternalBlendParams2);
		}
	}

	function OnZoomOut(int iSpeed)
	{
		local float OldZoomOut;

		OldZoomOut = _ArtilleryViewParam._CurrentZoomOut;

		_ArtilleryViewParam._CurrentZoomOut -= Abs(_ArtilleryViewParam._EagleEyeZoomSpeed);
		_ArtilleryViewParam._CurrentZoomOut = FClamp(_ArtilleryViewParam._CurrentZoomOut, _ArtilleryViewParam._MinZoomOut, _ArtilleryViewParam._MaxZoomOut);

		// SetViewTargetPos驱动的FOLLOW优先级更高，此时不设置ZOOM IN/ZOOM OUT的过渡参数
		if( _BlendTimeWhenFollow == 0.f && OldZoomOut != _ArtilleryViewParam._CurrentZoomOut )
		{
			BeginCameraFollow(_ArtilleryViewParam._InternalBlendParams2);
		}
	}

	// eagle view
	function ToggleView(int ActivePartType, int ViewModeList)
	{
		if(_PrevCameraStyle == 'FirstPersonView' || _PrevCameraStyle == 'TelescopeView')
		{
			SetCamMode('FirstPersonView',,_FirstViewParam._BlendParams);
		}
		else
		{
			SetCamMode('ThirdPersonView',,_FreeViewParam._BlendParams);
		}
	}
}

state FightJetView extends EagleEyeView
{
	function float CalcRealDistance(float ZDist, int Pitch)
	{
		return ZDist / Sin(Abs(Pitch) * UnrRotToRad);
	}

	function BeginState(name PreviousStateName)
	{
        local SeaUIFightController ui;
		local vector ValidLocation;
		local vector StartPos, EndPos, HitLoc;

        SeaPlayerControllerFight(PCOwner).ReserveCameraLookAt();
		
		CameraStyle = 'FightJetView';

		Super.BeginState(PreviousStateName);
		
		// 缓存未经角度修正的初始值
		if( _FightJetViewParam._CurrentZoomOut == 0 )
		{
			_FightJetViewParam._CurrentZoomOut = _FightJetViewParam._DefaultZoomOut;
		}
		_FightJetViewParam._MinZoomOutCached = _FightJetViewParam._MinZoomOut;
		_FightJetViewParam._MaxZoomOutCached = _FightJetViewParam._MaxZoomOut;
		_FightJetViewParam._DefaultZoomOutCached = _FightJetViewParam._DefaultZoomOut;
		// 修正MinZoomOut/MaxZoomOut,配置值没考虑PITCH
		_FightJetViewParam._MinZoomOut = CalcRealDistance(_FightJetViewParam._MinZoomOut, _FightJetViewParam._DefaultCamRotation.Pitch);
		_FightJetViewParam._MaxZoomOut = CalcRealDistance(_FightJetViewParam._MaxZoomOut, _FightJetViewParam._DefaultCamRotation.Pitch);
		_FightJetViewParam._DefaultZoomOut = CalcRealDistance(_FightJetViewParam._DefaultZoomOut, _FightJetViewParam._DefaultCamRotation.Pitch);

		// step 1. 获得有效位置数据
		ValidLocation = InitValidLocation();

		// step 2. push data to ViewTarget
		InitViewTargetParam(ValidLocation);

		// 前一状态判断为_bAlwaysLookAt，才需要进一步判断
		if( _bAlwaysLookAt )
		{
			StartPos = ViewTarget.POV.Location;
			EndPos = StartPos + Normal(Vector(ViewTarget.POV.Rotation)) * MAX_TRACE_RADIUS;
			_bAlwaysLookAt = false;
			if( SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0.f, HitLoc) )
			{
				// 前后两次判断容差在一定范围才认为相同
				if( Abs(HitLoc.X - _PointLookAt.X) < _ToleranceDistance && Abs(HitLoc.Y - _PointLookAt.Y) < _ToleranceDistance )
				{
					_bAlwaysLookAt = true;
					_PointLookAt = HitLoc;
				}
			}
		}

        //  进入航母鹰眼视角，隐藏血量速度
        ui = SeaUIFightController(`SeaUIController);
        if (ui != none)
        {
            ui._CrossHair.Hide();
			ui._WeaponView._SwitchFireMode.SetVisible(false);
        }
		
		`SeaGameViewportClient.ShowCursor(true, true, true);
		`SeaGameViewportClient.SetScreenDragFlag(true); // 鹰眼视角下，鼠标可拖拽屏幕
	}
	
	function EndState(name NextStateName)
	{
        local SeaPlayerControllerFight PC;
        local SeaUIFightController ui;
        local SeaSquadron Squad;
		local ClientPlayerData curPlayerData;
		local ShipData ship;
		
		curPlayerData = `ClientWorldInfo.GetCurWatchPlayerData();
		if (curPlayerData == none)
		{
			return;
		}
		ship = curPlayerData.GetFightingShip();
		if (ship == none)
		{
			return;
		}
        PC = SeaPlayerControllerFight(PCOwner);
        if (PC != none)
        {
            Squad = ship.GetActiveSquad();
            if (Squad != none && Squad._InAction && Squad._TargetSpec == ETS_Setting)
            {
                Squad.CancelTargetSetting();
			    PC.ResetSquadTargetMark();
            }
        }

        //  离开航母鹰眼视角，显示血量和速度
        ui = SeaUIFightController(`SeaUIController);
        if (ui != none && !PCOwner.IsInState('Dead') )
        {
			if( ship._ActivePartType == -1 )
			{ // 仍选择舰载机时隐藏准心
				ui._CrossHair.HideCross();
			}
			else
			{
				ui._CrossHair.ShowCross();
			}
            ui._CrossHair.Show();
			ui._WeaponView._SwitchFireMode.SetVisible(true);
        }

		// 还原初始值
		_FightJetViewParam._MinZoomOut = _FightJetViewParam._MinZoomOutCached;
		_FightJetViewParam._MaxZoomOut = _FightJetViewParam._MaxZoomOutCached;
		_FightJetViewParam._DefaultZoomOut = _FightJetViewParam._DefaultZoomOutCached;

		Super.EndState(NextStateName);
		
		`SeaGameViewportClient.ShowCursor(false);
		`SeaGameViewportClient.SetScreenDragFlag(false); // 离开鹰眼视角，鼠标不可拖拽屏幕
	}

	// 获得合法的起始目标点
	// @param StartPos	观察位置
	// @param Dir		观察朝向
	// @param MinBound/MaxBound/MaxHeight	最小边界值/最大边界值/最大高度
	function vector GetValidLocation(vector StartPos, vector Dir, vector2D MinBound, vector2D MaxBound, float MaxHeight)
	{
		local vector EndPos;
		local vector HitLoc;
		local bool bTraceAnything;

		EndPos = StartPos + Dir * MAX_TRACE_RADIUS;
		bTraceAnything = SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0, HitLoc, true);

		// 检测不到物体时使用起始点计算
		ClampToBoundArea(HitLoc, bTraceAnything ? HitLoc : StartPos, MinBound, MaxBound, vect2d(-50, -50));

		return HitLoc;
	}

	// 获得有效位置数据
	function vector InitValidLocation()
	{
		local vector ValidLocation;			// 切换至鹰眼的初始合法位置
		local vector StartPos;
		local vector CameraDir;
		
		CameraDir = Vector(_FightJetViewParam._DefaultCamRotation);

		StartPos = (_bUseSpecifiedLookAtPoint ? _SpecifiedLookAtPoint : ViewTarget.Target.Location) - CameraDir * _FightJetViewParam._MaxZoomOut;
		
		ValidLocation = GetValidLocation(StartPos, CameraDir, _MinBound, _MaxBound, _MaxSceneHeight);
		return ValidLocation;
	}

	function InitViewTargetParam(vector ValidLoc)
	{
		ViewTarget.POV.Rotation = _FightJetViewParam._DefaultCamRotation;
		ViewTarget.POV.Location = ValidLoc;
		ViewTarget.POV.Location -= _FightJetViewParam._CurrentZoomOut * Normal(Vector(_FightJetViewParam._DefaultCamRotation));
		ViewTarget.POV.FOV = _FightJetViewParam._DefaultFOV;
	}
	
	function SetViewTargetPos(Vector2D InLookAt)
	{
		local vector StartPos, EndPos;
		local vector HitLoc;
		local vector CameraDir;

		CameraDir = Vector(_FightJetViewParam._DefaultCamRotation);

		StartPos.X = InLookAt.X;
		StartPos.Y = InLookAt.Y;
		StartPos -= CameraDir * _FightJetViewParam._MaxZoomOut;
		EndPos = StartPos + Normal(Vector(_FightJetViewParam._DefaultCamRotation)) * MAX_TRACE_RADIUS;

		// 检测与场景的碰撞点，忽略空气墙
		if( !SeaPlayerControllerFight(PCOwner).TargetTrace(StartPos, EndPos, true, HitLoc) )
		{
			HitLoc = StartPos;
			HitLoc.Z = 0;
		}

		// 把碰撞点限制在bound volume内
		ClampToBoundArea(HitLoc, HitLoc, _MinBound, _MaxBound, vect2d(-50,-50));

		InitViewTargetParam(HitLoc);
	
		BeginCameraFollow(_FightJetViewParam._InternalBlendParams);
	}

	function UpdateVT(out TViewTarget OutVT, float DeltaTime)
	{
		local vector HitLocation;
		local vector2D CameraDelta;
		local vector CameraDir;
		local vector StartPos;

		super.UpdateVT(OutVT, DeltaTime);
		
		CameraDelta.X = _MouseDeltaByDrag.Y * _FightJetViewParam._EagleEyeMoveSpeed;
		CameraDelta.Y = _MouseDeltaByDrag.X * _FightJetViewParam._EagleEyeMoveSpeed;
				
		CameraDir = Vector(_FightJetViewParam._DefaultCamRotation);
		
		// 1.确定camera方向与场景的碰撞点
		StartPos = OutVT.POV.Location;
		SeaPlayerControllerFight(PCOwner).TargetTrace(StartPos, CameraDir, true, HitLocation);	// 不检测空气墙
		
		// 2.移动碰撞点（DeltaXY)，限制在BoundVolume内
		HitLocation.X += CameraDelta.X;
		HitLocation.Y += CameraDelta.Y;
		ClampToBoundArea(HitLocation, HitLocation, _MinBound, _MaxBound, vect2d(0,0));

		// 3.基于新的位置再次检测新的碰撞点
		StartPos = HitLocation - CameraDir * _FightJetViewParam._MaxZoomOut;
		SeaPlayerControllerFight(PCOwner).TargetTrace(StartPos, CameraDir, true, HitLocation);

		OutVT.POV.Location = HitLocation - CameraDir * _FightJetViewParam._CurrentZoomOut;
		OutVT.POV.Rotation = _FightJetViewParam._DefaultCamRotation;
	}
	
	function OnZoomIn(int iSpeed)
	{
		local float OldZoomOut;

		OldZoomOut = _FightJetViewParam._CurrentZoomOut;

		_FightJetViewParam._CurrentZoomOut += Abs(_FightJetViewParam._EagleEyeZoomSpeed);
		_FightJetViewParam._CurrentZoomOut = FClamp(_FightJetViewParam._CurrentZoomOut, _FightJetViewParam._MinZoomOut, _FightJetViewParam._MaxZoomOut);
		
		// SetViewTargetPos驱动的FOLLOW优先级更高，此时不设置ZOOM IN/ZOOM OUT的过渡参数
		if( _BlendTimeWhenFollow == 0.f && OldZoomOut != _FightJetViewParam._CurrentZoomOut )
		{
			BeginCameraFollow(_FightJetViewParam._InternalBlendParams2);
		}
    }

	function OnZoomOut(int iSpeed)
	{
		local float OldZoomOut;

		OldZoomOut = _FightJetViewParam._CurrentZoomOut;

		_FightJetViewParam._CurrentZoomOut -= Abs(_FightJetViewParam._EagleEyeZoomSpeed);
		_FightJetViewParam._CurrentZoomOut = FClamp(_FightJetViewParam._CurrentZoomOut, _FightJetViewParam._MinZoomOut, _FightJetViewParam._MaxZoomOut);

		// SetViewTargetPos驱动的FOLLOW优先级更高，此时不设置ZOOM IN/ZOOM OUT的过渡参数
		if( _BlendTimeWhenFollow == 0.f && OldZoomOut != _FightJetViewParam._CurrentZoomOut )
		{
			BeginCameraFollow(_FightJetViewParam._InternalBlendParams2);
		}
	}

	// fight jet view
	function ToggleView(int ActivePartType, int ViewModeList)
	{
		if(_PrevCameraStyle == 'FirstPersonView' || _PrevCameraStyle == 'TelescopeView')
		{
			SetCamMode('FirstPersonView',,_FirstViewParam._BlendParams);
		}
		else
		{
			SetCamMode('ThirdPersonView', true, _FreeViewParam._BlendParams);
		}
	}
}


//////////////////////////// 鸟瞰模式
state BirdView
{
	// 鸟瞰模式的初始位置即狙击模式时的位置
	function ModifyCameraLoc(out vector InCameraLoc, float Height)
	{
		InCameraLoc.Z = _FirstViewParam._FirstPersonViewOffset * Height 
						+ _TelescopeViewParam._TelescopeViewOffset[Max(0, _TelescopeViewParam._TelescopeViewOffset.Length - 1)] 
						+ _OceanZ;
	}

	function BeginState(name PreviousStateName)
	{
		local vector HitLoc;
		local vector TargetLoc;
		local rotator TargetRot;
		local float ZRelativeOcean;	// 相对海平面的高度
		local float UpperRange, LowerRange;
		local vector StartPos, EndPos;
		local vector LookDir;
		local vector LookAtPoint;

		_PrevCameraStyle = PreviousStateName;
		CameraStyle = 'BirdView';

        global.BeginState(PreviousStateName);

		// 初始化鸟瞰模式OriginalZ，相对海平面的高度
		ViewTarget.Target.GetActorEyesViewPoint(TargetLoc, TargetRot);
		ZRelativeOcean = TargetLoc.Z - _OceanZ;

		// 检测配置参数的有效性
		_BirdViewParam._MinAngle = Abs(_BirdViewParam._MinAngle) % 90.f;
		_BirdViewParam._MinAngle = FMax(1.f, _BirdViewParam._MinAngle);		// 避免MinAngle为0
		_BirdViewParam._MaxAngle = FMax(_BirdViewParam._MinAngle, _BirdViewParam._MaxAngle);
		_BirdViewParam._MaxHeight = FMax(ZRelativeOcean + 100, _BirdViewParam._MaxHeight);
		_BirdViewParam._ScaleOfAscend = FMax(0.01f, _BirdViewParam._ScaleOfAscend);
		_BirdViewParam._ScaleOfBeyond = FMax(0.01f, _BirdViewParam._ScaleOfBeyond);
		_BirdViewParam._CurrBirdLevel = FMax(0, _BirdViewParam._CurrBirdLevel);

		// 计算XY1,XY2,XY3，并检测参数之间的有效性
		// _DistanceXY3 >= _DistanceXY2 >= _DistanceXY1
		_BirdViewParam._DistanceXY1 = ZRelativeOcean / Tan(_BirdViewParam._MinAngle * DegToRad);
		_BirdViewParam._DistanceXY2 = (_BirdViewParam._MaxHeight - _OceanZ) / Tan(_BirdViewParam._MinAngle * DegToRad);
		
		SeaPlayerControllerFight(PCOwner)._PlayerData._FightingShip.GetActiveWeaponRange(UpperRange, LowerRange);
		_BirdViewParam._DistanceXY3 = UpperRange == 0.f ? (_MaxBound.X - _MinBound.X) * 0.5f : UpperRange;
		
		_BirdViewParam._DistanceXY2 = FMax(_BirdViewParam._DistanceXY2, _BirdViewParam._DistanceXY1);
		_BirdViewParam._DistanceXY3 = FMax(_BirdViewParam._DistanceXY3, _BirdViewParam._DistanceXY2);
		
		if( _bUseSpecifiedLookAtPoint )
		{
			LookAtPoint = _SpecifiedLookAtPoint;
		}
		else
		{
			StartPos = CameraCache.POV.Location;
			LookDir = Normal(Vector(CameraCache.POV.Rotation));
			EndPos = StartPos + LookDir * MAX_TRACE_RADIUS;
			if( !SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0.f, HitLoc) )
			{
				HitLoc = CameraCache.POV.Location + LookDir * MAX_TRACE_RADIUS;
			}
			LookAtPoint = HitLoc;
		}
		// 根据当前POV计算第一帧ViewTarget.POV
		SetViewTargetPos3D(LookAtPoint);

		// 前一状态判断为_bAlwaysLookAt，才需要进一步判断
		if( _bAlwaysLookAt )
		{
			StartPos = ViewTarget.POV.Location;
			EndPos = StartPos + Normal(Vector(ViewTarget.POV.Rotation)) * MAX_TRACE_RADIUS;
			_bAlwaysLookAt = false;
			if( SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0.f, HitLoc) )
			{
				// 前后两次判断容差在一定范围才认为相同
				if( Abs(HitLoc.X - _PointLookAt.X) < _ToleranceDistance && Abs(HitLoc.Y - _PointLookAt.Y) < _ToleranceDistance )
				{
					_bAlwaysLookAt = true;
					_PointLookAt = HitLoc;
				}
			}
		}

		if( !_bEnableBlend )
		{ // 无过渡时
			// 显示小地图可视区域
			SeaUIFightController(`SeaUIController).ShowMiniMapViewArea(true);
		}

		// PVP新手教程事件
		`ClientWorldInfo.InputPVPEvent(EPVPTE_EagleEyeView);
	}

	function EndState(name NextStateName)
	{
		local SeaUIFightController UIFight;

		UIFight = SeaUIFightController(`SeaUIController);
		if( UIFight != none )
        {
			UIFight.ShowMiniMapViewArea(false);
            UIFight._AimingView.SwitchAimingLine(false);
        }

		Super.EndState(NextStateName);
	}

	function OnEndCameraSwitch()
	{
        local SeaUIFightController ui;

        ui = SeaUIFightController(`SeaUIController);
        if (ui == none)
            return;

		ui.ShowMiniMapViewArea(true);           //  显示小地图可视区域
        ui._AimingView.SwitchAimingLine(true);  //  显示炮线
	}

	function bool PointInTriangle(vector P, vector A, vector B, vector C)
	{
		local vector v0, v1, v2;
		local float dot00, dot01, dot02, dot11, dot12;
		local float inverDeno;
		local float u, v;

		v0 = C - A ;
		v1 = B - A ;
		v2 = P - A ;

		dot00 = v0 Dot v0;
		dot01 = v0 Dot v1;
		dot02 = v0 Dot v2;
		dot11 = v1 Dot v1;
		dot12 = v1 Dot v2;

		inverDeno = 1 / (dot00 * dot11 - dot01 * dot01) ;

		u = (dot11 * dot02 - dot01 * dot12) * inverDeno ;
		if (u < 0 || u > 1) // if u out of range, return directly
		{
			return false ;
		}

		v = (dot00 * dot12 - dot01 * dot02) * inverDeno ;
		if (v < 0 || v > 1) // if v out of range, return directly
		{
			return false ;
		}

		return u + v <= 1 ;
	}
	
	// InLookAt,VertexA,VertexB1,VertexC1,VertexB2,VertexC2均在同一平面
	// 返回在此区间与Ocean的交点
	function vector DetermineWhichTriangle(vector InLookAt, vector VertexA, vector VertexB1, vector VertexC1, vector VertexB2, vector VertexC2, optional bool bLimitBound=true, optional bool bLimitRange=false)
	{
		local vector LookDir;
		local vector HitLoc;
		local rotator LookRot;
		local vector PointLookAtOcean;
		local vector HitBound;
		local float HitTime;
		local vector StartPos, EndPos;
		local float TraceLength;
		local vector ClampToPos;

		// 判断InLookAt在哪个区间内,与Ocean的交点
		if( PointInTriangle(InLookAt, VertexA, VertexB1, VertexC1) )
		{ // 处于区间1
			_BirdViewParam._PointInWhere = 1;

			LookRot = Rotator(InLookAt - VertexB1);
			LookDir = Normal(Vector(LookRot));
			if( SeaPlayerControllerFight(PCOwner).OceanTrace(InLookAt - LookDir * 10.f, LookDir, HitLoc) )
			{
				PointLookAtOcean = HitLoc;
				PointLookAtOcean.Z = _OceanZ;
			}
			else
			{
				PointLookAtOcean = VertexC1;
				PointLookAtOcean.Z = _OceanZ;
			}
		}
		else
		{
			if( PointInTriangle(InLookAt, VertexA, VertexB2, VertexC2) )
			{ // 处于区间2
				_BirdViewParam._PointInWhere = 2;
			}
			else
			{ // 处于区间3
				_BirdViewParam._PointInWhere = 3;
			}

			LookRot = Rotator(InLookAt - VertexA);
			LookRot.Pitch = _BirdViewParam._MinAngle * DegToUnrRot * -1;
			LookDir = Normal(Vector(LookRot));
			if( SeaPlayerControllerFight(PCOwner).OceanTrace(InLookAt - LookDir * 10.f, LookDir, HitLoc) )
			{
				PointLookAtOcean = HitLoc;
				PointLookAtOcean.Z = _OceanZ;
			}
			else
			{
				LookRot.Pitch = 0;
				LookDir = Normal(Vector(LookRot));
				PointLookAtOcean = VertexA + LookDir * (_MaxBound.X - _MinBound.X) * 2.f;
				PointLookAtOcean.Z = _OceanZ;
			}
		}

		if( bLimitBound )
		{ // 与Ocean交点限制在区域内
			if( _BirdViewParam._PointInWhere == 1 )
			{
				LookDir = PointLookAtOcean - VertexA;
				LookDir.Z = 0.f;
				LookDir = Normal(LookDir);
				StartPos = VertexA;
				TraceLength = (_MaxBound.X - _MinBound.X);
				EndPos = StartPos + LookDir * TraceLength;
				if( LineExtentBoundIntersection(HitBound, HitTime, StartPos, EndPos, _MinBound, _MaxBound, vect2d(-20, -20)) )
				{
					PointLookAtOcean = StartPos + LookDir * FMin(HitTime * TraceLength, _BirdViewParam._DistanceXY1);
					PointLookAtOcean.Z = _OceanZ;
				}
				else
				{
					PointLookAtOcean = VertexC1;
					PointLookAtOcean.Z = _OceanZ;
				}
			}
			else
			{
				// 限制PointLookAtOcean在Bound内
				if( !ClampToBoundArea(ClampToPos, PointLookAtOcean, _MinBound, _MaxBound, vect2d(-20.f, -20.f)) )
				{
					StartPos = VertexA;
					EndPos = PointLookAtOcean;
					if( LineExtentBoundIntersection(HitBound, HitTime, StartPos, EndPos, _MinBound, _MaxBound, vect2d(-20, -20)) && HitTime < 1.f )
					{
						HitBound.Z = _OceanZ + 0.01f;
						PointLookAtOcean = HitBound;
					}
				}
			}
		}

		// 限制在最远射程内
		if( bLimitRange )
		{
			if( VSize2D(PointLookAtOcean - VertexA) > _BirdViewParam._DistanceXY3 )
			{
				LookDir = PointLookAtOcean - VertexA;
				LookDir.Z = 0.f;
				LookDir = Normal(LookDir);
				PointLookAtOcean = VertexA + LookDir * _BirdViewParam._DistanceXY3;
				PointLookAtOcean.Z = _OceanZ + 0.01f;
			}
		}

		if( bLimitBound || bLimitRange )
		{
			// 重新计算_PointInWhere
			PointLookAtOcean = DetermineWhichTriangle(PointLookAtOcean, VertexA, VertexB1, VertexC1, VertexB2, VertexC2, false, false);
		}

		_BirdViewParam._DistanceXY = VSize2D(PointLookAtOcean - VertexA);
		_BirdViewParam._DistanceXY = FMax(_BirdViewParam._DistanceXY, _BirdViewParam._DistanceXY1);

		return PointLookAtOcean;
	}

	function SetViewTargetPos3D(vector InLookAt)
	{
		local vector Offset;
		local vector TargetLoc;
		local rotator TargetRot;
		local vector LookDir;
		local vector VertexA, VertexB1, VertexC1, VertexB2, VertexC2;
		local rotator LookRot;
		local vector PointLookAtOcean;
		local bool bLimitRange;
		local vector ClampToPos;

		ViewTarget.Target.GetActorEyesViewPoint(TargetLoc, TargetRot);
		
		// 计算两个三角形顶点
		VertexA = TargetLoc;
		VertexA.Z = _OceanZ;

		VertexB1 = TargetLoc;

		LookDir = InLookAt - VertexA;
		LookDir.Z = 0;
		LookDir = Normal(LookDir);
		VertexC1 = VertexA + LookDir * _BirdViewParam._DistanceXY1;

		VertexB2 = TargetLoc;
		VertexB2.Z = _BirdViewParam._MaxHeight;

		VertexC2 = VertexA + LookDir * _BirdViewParam._DistanceXY2;

		////////// step 1.	确定目标点在哪个区间
		InLookAt.Z += 0.01f;		// 防止InLookAt在CA上引起的计算误差
		bLimitRange = !ClampToBoundArea(ClampToPos, InLookAt, _MinBound, _MaxBound, vect2d(-20.f, -20.f));		// InLookAt在活动范围内则不限制最远射程
		PointLookAtOcean = DetermineWhichTriangle(InLookAt, VertexA, VertexB1, VertexC1, VertexB2, VertexC2, true, bLimitRange);

		////////// step 2.	calc offset
		if( _BirdViewParam._PointInWhere == 2 )
		{
			Offset.X = 0.f;
			Offset.Y = 0.f;
			Offset.Z = Tan(_BirdViewParam._MinAngle * DegToRad) * _BirdViewParam._DistanceXY - TargetLoc.Z;
		}
		else if( _BirdViewParam._PointInWhere == 3 )
		{
			Offset.X = 0.f;
			Offset.Y = 0.f;
			Offset.Z = _BirdViewParam._MaxHeight - TargetLoc.Z;

			LookRot = Rotator(PointLookAtOcean - VertexA);
			LookRot.Pitch = 0;
			LookDir = Normal(Vector(LookRot));

			Offset += LookDir * (_BirdViewParam._DistanceXY - _BirdViewParam._DistanceXY2);
		}

		/////////// step 3.	update vt
		// 1. update location
		ViewTarget.POV.Location = TargetLoc + Offset;

		// 2. update rotation
		ViewTarget.POV.Rotation = Rotator(PointLookAtOcean - TargetLoc);
		if( _BirdViewParam._PointInWhere == 2 || _BirdViewParam._PointInWhere == 3 )
		{
			ViewTarget.POV.Rotation.Pitch = _BirdViewParam._MinAngle * DegToUnrRot * -1;
		}

		// 3. update fov
		ViewTarget.POV.FOV = _BirdViewParam._BirdZoomLevel[_BirdViewParam._CurrBirdLevel];

		// 确定最终VT.Rotation后立即同步到PC
		PCOwner.SetRotation(ViewTarget.POV.Rotation);
	}

	function SetViewTargetPos(vector2D InLookAt)
	{
		local vector StartPos, EndPos;
		local vector HitLoc;

		StartPos.X = InLookAt.X;
		StartPos.Y = InLookAt.Y;
		StartPos.Z = _MaxSceneHeight;
		ClampToBoundArea(StartPos, StartPos, _MinBound, _MaxBound, vect2d(-100,-100));

		EndPos = StartPos + vect(0,0,-1) * MAX_TRACE_RADIUS;
		if( !SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0.f, HitLoc) )
		{
			HitLoc = StartPos;
		}

		SetViewTargetPos3D(HitLoc);

		BeginCameraFollow(_BirdViewParam._InternalBlendParams2);
	}

	function UpdateVT(out TViewTarget OutVT, float DeltaTime)
	{
		local Rotator CameraRot;
		local float Delta;
		local vector TargetLoc;
		local rotator TargetRot;
		local int DeltaYaw;
		local vector VertexA, VertexB1, VertexC1, VertexB2, VertexC2;
		local vector LookDir;
		local vector PointLookAtOcean, PendingPointLookAtOcean;

		Super.UpdateVT(OutVT, DeltaTime);

		ViewTarget.Target.GetActorEyesViewPoint(TargetLoc, TargetRot);
		ModifyCameraRotation(DeltaTime, TargetRot);

		if( _BirdViewParam._PointInWhere == 1 )
		{
			// 计算两个三角形顶点
			VertexA = TargetLoc;
			VertexA.Z = _OceanZ;

			VertexB1 = TargetLoc;

			CameraRot = TargetRot;
			CameraRot.Pitch = 0;
			LookDir = Normal(Vector(CameraRot));

			VertexC1 = VertexA + LookDir * _BirdViewParam._DistanceXY1;

			VertexB2 = TargetLoc;
			VertexB2.Z = _BirdViewParam._MaxHeight;

			VertexC2 = VertexA + LookDir * _BirdViewParam._DistanceXY2;
		
			// 检测是否超出Bound限制
			PendingPointLookAtOcean = VertexA + LookDir * _BirdViewParam._DistanceXY;

			// 重新计算_PointInWhere
			PointLookAtOcean = DetermineWhichTriangle(PendingPointLookAtOcean, VertexA, VertexB1, VertexC1, VertexB2, VertexC2);
			
			OutVT.POV.Rotation = Rotator(PointLookAtOcean - TargetLoc);
			OutVT.POV.Location = TargetLoc;
		}
		else if( _BirdViewParam._PointInWhere == 2 )
		{
			// 计算两个三角形顶点
			VertexA = TargetLoc;
			VertexA.Z = _OceanZ;

			VertexB1 = TargetLoc;

			CameraRot = TargetRot;
			CameraRot.Pitch = 0;
			LookDir = Normal(Vector(CameraRot));

			VertexC1 = VertexA + LookDir * _BirdViewParam._DistanceXY1;

			VertexB2 = TargetLoc;
			VertexB2.Z = _BirdViewParam._MaxHeight;

			VertexC2 = VertexA + LookDir * _BirdViewParam._DistanceXY2;
		

			Delta = _MouseDelta.Y * _BirdViewParam._ScaleOfAscend;

			_BirdViewParam._DistanceXY += Delta;

			// 检测是否超出Bound限制
			PendingPointLookAtOcean = VertexA + LookDir * _BirdViewParam._DistanceXY;

			// 重新计算_PointInWhere
			PointLookAtOcean = DetermineWhichTriangle(PendingPointLookAtOcean, VertexA, VertexB1, VertexC1, VertexB2, VertexC2);

			if( _BirdViewParam._PointInWhere == 1 )
			{
				OutVT.POV.Rotation = Rotator(PointLookAtOcean - TargetLoc);
				OutVT.POV.Location = TargetLoc;
			}
			else
			{
				OutVT.POV.Rotation = TargetRot;
				OutVT.POV.Rotation.Pitch = _BirdViewParam._MinAngle * DegToUnrRot * -1;
				OutVT.POV.Location = TargetLoc;
				OutVT.POV.Location.Z = Tan(_BirdViewParam._MinAngle * DegToRad) * FMin(_BirdViewParam._DistanceXY, _BirdViewParam._DistanceXY2);
			}
		}
		else if( _BirdViewParam._PointInWhere == 3 )
		{
			Delta = _MouseDelta.Y * _BirdViewParam._ScaleOfBeyond;			
			_BirdViewParam._DistanceXY += Delta;

			CameraRot = TargetRot;
			CameraRot.Yaw -= SeaPlayerControllerFight(PCOwner)._DeltaRot.Yaw;		// YAW不受鼠标控制，还原之前值
			CameraRot.Pitch = 0;

			DeltaYaw = Atan(_MouseDelta.X * _BirdViewParam._ScaleOfBeyond / (_BirdViewParam._DistanceXY - _MouseDelta.Y * _BirdViewParam._ScaleOfBeyond)) * RadToUnrRot;

			CameraRot.Yaw += DeltaYaw;
			LookDir = Normal(Vector(CameraRot));


			// 计算两个三角形顶点
			VertexA = TargetLoc;
			VertexA.Z = _OceanZ;

			VertexB1 = TargetLoc;

			VertexC1 = VertexA + LookDir * _BirdViewParam._DistanceXY1;

			VertexB2 = TargetLoc;
			VertexB2.Z = _BirdViewParam._MaxHeight;

			VertexC2 = VertexA + LookDir * _BirdViewParam._DistanceXY2;
		
			// 检测是否超出Bound限制
			PendingPointLookAtOcean = VertexA + LookDir * _BirdViewParam._DistanceXY;

			// 重新计算_PointInWhere
			PointLookAtOcean = DetermineWhichTriangle(PendingPointLookAtOcean, VertexA, VertexB1, VertexC1, VertexB2, VertexC2);

			if( _BirdViewParam._PointInWhere == 3 )
			{
				OutVT.POV.Location = TargetLoc;
				OutVT.POV.Location.Z = _BirdViewParam._MaxHeight;
				OutVT.POV.Location += LookDir * (_BirdViewParam._DistanceXY - _BirdViewParam._DistanceXY2);
			
				OutVT.POV.Rotation = CameraRot;
				OutVT.POV.Rotation.Pitch = _BirdViewParam._MinAngle * DegToUnrRot * -1;
			}
			else
			{
				OutVT.POV.Rotation = TargetRot;
				OutVT.POV.Rotation.Pitch = _BirdViewParam._MinAngle * DegToUnrRot * -1;
				OutVT.POV.Location = TargetLoc;
				OutVT.POV.Location.Z = Tan(_BirdViewParam._MinAngle * DegToRad) * _BirdViewParam._DistanceXY2;
			}
		}

		OutVT.POV.FOV = _BirdViewParam._BirdZoomLevel[_BirdViewParam._CurrBirdLevel];
				
		PCOwner.SetRotation(OutVT.POV.Rotation);
	}
		
	function int SetZoomLevel(int iLevel)
	{
		local int OldLevel;

		OldLevel = _BirdViewParam._CurrBirdLevel;

		_BirdViewParam._CurrBirdLevel = iLevel;
		_BirdViewParam._CurrBirdLevel = Clamp(_BirdViewParam._CurrBirdLevel, 0, _BirdViewParam._BirdZoomLevel.Length-1);

		if( OldLevel != _BirdViewParam._CurrBirdLevel )
		{
			BeginCameraFollow(_BirdViewParam._InternalBlendParams);
		}

		return _BirdViewParam._CurrBirdLevel;
	}

	function OnZoomOut(int iSpeed)
	{
		SetZoomLevel(_BirdViewParam._CurrBirdLevel+1);
	}

	function OnZoomIn(int iSpeed)
	{
		SetZoomLevel(_BirdViewParam._CurrBirdLevel-1);
	}

	function ToggleView(int ActivePartType, int ViewModeList)
	{
		SetCamMode('ThirdPersonView',,_FreeViewParam._BlendParams);		
	}
}


/////////////////////////// 自由摄像机模式，仅编辑器模式使用
function SwapTarget();
state FreeCameraView
{
	function BeginState(name PreviousStateName)
	{
		_PrevCameraStyle = PreviousStateName;
		CameraStyle = 'FirstPersonView';

        global.BeginState(PreviousStateName);

        _EditorModeParam._TraceTarget = none;
        _EditorModeParam._FocusPartType = 0;
        _EditorModeParam._FocusPartIndex = 0;

        if (PreviousStateName == 'ArtilleryView')
        {
            LookAtTarget();
        }
        else if(PreviousStateName=='FightJetView')
        {
            UseViewTargetRotation();
        }

        SetFOV(_EditorModeParam._DefaultFOV);
        SeaPlayerControllerFight(PCOwner).ClearFreeCameraFlags();
	}

    function EndState(name NextStateName)
	{
        _EditorModeParam._TraceTarget = none;
        _EditorModeParam._FocusPartType = 0;
        _EditorModeParam._FocusPartIndex = 0;
		Super.EndState(NextStateName);
	} 

	function UpdateVT(out TViewTarget OutVT, float DeltaTime)
	{
        local vector LookAt, Right, CamUp, Loc, PendingLoc;
        local vector TargetLoc;
        local float NewFOV;
        local SeaPlayerControllerFight Player;
        local SeaPawn TargetPawn;
        local SeaSkeletalMeshComponent MeshComp;
        local ShipData TargetData;
        local AttachmentData AttachData;
        local array<int> AttachmentList;

        Player = SeaPlayerControllerFight(PCOwner);
        if (Player == none)
            return;

		super.UpdateVT(OutVT, DeltaTime);

        if (Player._PitchUpPressed)
            PCOwner.PlayerInput.aLookUp -= _EditorModeParam._RotateRatio * 0.01;
        else if (Player._PitchDownPressed)
            PCOwner.PlayerInput.aLookUp += _EditorModeParam._RotateRatio * 0.01;
        if (Player._YawCWPressed)
            PCOwner.PlayerInput.aTurn += _EditorModeParam._RotateRatio * 0.01;
        else if (Player._YawCCWPressed)
            PCOwner.PlayerInput.aTurn -= _EditorModeParam._RotateRatio * 0.01;

        OutVT.Target.GetActorEyesViewPoint(Loc, OutVT.POV.Rotation);
        GetAxes(OutVT.POV.Rotation, LookAt, Right, CamUp);

        if (_EditorModeParam._TraceTarget != none)
        {
            if (_EditorModeParam._FocusPartType == 0 || _EditorModeParam._TraceTarget.IsA('SeaAircraft'))
            {
                TargetLoc = _EditorModeParam._TraceTarget.Location;
            }
            else
            {
                TargetLoc = _EditorModeParam._TraceTarget.Location;  //  for default

                TargetPawn = SeaPawn(_EditorModeParam._TraceTarget);
                if (TargetPawn != none)
                {
                    TargetData = SeaPlayerControllerFight(TargetPawn.Controller)._PlayerData._FightingShip;

                    foreach TargetData._AttachmentDataList(AttachData)
                    {
                        if (AttachData._PartType == _EditorModeParam._FocusPartType)
                            AttachmentList.AddItem(AttachData._AttachedId);
                    }

                    if (AttachmentList.Length > 0)
                    {
                        if (_EditorModeParam._FocusPartIndex >= AttachmentList.Length)
                            _EditorModeParam._FocusPartIndex = 0;
                        
                        MeshComp = TargetPawn.GetAttachedMeshComponent(AttachmentList[_EditorModeParam._FocusPartIndex]);
                        if (MeshComp != none)
                        {
                            TargetLoc = MeshComp.GetPosition();
                            TargetLoc.Z += MeshComp.SkeletalMesh.Bounds.BoxExtent.Z * 2;
                        }
                    }
                }
            }

            if (_EditorModeParam._Distance < 0)
                _EditorModeParam._Distance = VSize(TargetLoc - OutVT.POV.Location);

            _EditorModeParam._Distance -= _EditorModeParam._TranslateRatio * _RadialMoveFlag;

            LookAt = vect(1,0,0) >> OutVT.POV.Rotation;
            PendingLoc = TargetLoc - LookAt * _EditorModeParam._Distance;
        }
        else
        {
            _EditorModeParam._Distance = -1;
            PendingLoc = OutVT.POV.Location;
            PendingLoc += LookAt * (_EditorModeParam._TranslateRatio * _RadialMoveFlag);
            PendingLoc += Right * (_EditorModeParam._TranslateRatio * _SideMoveFlag);
            PendingLoc.Z += _EditorModeParam._TranslateRatio * _VerticalMoveFlag;
        }

        OutVT.POV.Location = PendingLoc;
        //OutVT.POV.Location = VInterpTo(OutVT.POV.Location, PendingLoc, DeltaTime, 4);

        NewFOV = GetFOVAngle();
        if (_FOVZoomFlag != 0)
        {
            NewFOV += _EditorModeParam._ZoomRatio * _FOVZoomFlag;
            NewFOV = Clamp(NewFOV, _EditorModeParam._MinFOV, _EditorModeParam._MaxFOV);
            SetFOV(NewFOV);
        }
        else if (_EditorModeParam._AutoReverseFOV)
        {
            //  自动回复FOV
            if (NewFOV > _EditorModeParam._DefaultFOV)
            {
                NewFOV -= _EditorModeParam._ZoomRatio;
                NewFOV = Max(NewFOV, _EditorModeParam._DefaultFOV);
                SetFOV(NewFOV);
            }
            else if (NewFOV < _EditorModeParam._DefaultFOV)
            {
                NewFOV += _EditorModeParam._ZoomRatio;
                NewFOV = Min(NewFOV, _EditorModeParam._DefaultFOV);
                SetFOV(NewFOV);
            }
        }
	}

	function OnZoomOut(int iSpeed)
	{
        local SeaPlayerControllerFight Player;

        Player = SeaPlayerControllerFight(PCOwner);
        if (Player == none)
            return;

        if (Player._AdjustRotSpeed ||
            Player._PitchUpPressed || Player._PitchDownPressed ||
            Player._YawCWPressed || Player._YawCCWPressed)
        {
		    if (_EditorModeParam._RotateRatio < _EditorModeParam._MaxRotRatio)
                _EditorModeParam._RotateRatio++;
        }
        else
        {
		    if (_EditorModeParam._TranslateRatio < _EditorModeParam._MaxTransRatio)
                _EditorModeParam._TranslateRatio++;
        }
	}

	function OnZoomIn(int iSpeed)
	{
        local SeaPlayerControllerFight Player;

        Player = SeaPlayerControllerFight(PCOwner);
        if (Player == none)
            return;

        if (Player._AdjustRotSpeed ||
            Player._PitchUpPressed || Player._PitchDownPressed ||
            Player._YawCWPressed || Player._YawCCWPressed)
        {
		    if (_EditorModeParam._RotateRatio > _EditorModeParam._MinRotRatio)
                _EditorModeParam._RotateRatio--;
        }
        else
        {
		    if (_EditorModeParam._TranslateRatio > _EditorModeParam._MinTransRatio)
                _EditorModeParam._TranslateRatio--;
        }
	}

    function SwapTarget()
    {
        //local SeaPawn Ship;
        //local ClientPlayerData PlayerData;
        local PendingAircraft AircraftData;
        local SeaAircraft Aircraft;
        local int idx;

        if (_EditorModeParam._TraceTarget != none)
        {
            if (_EditorModeParam._TraceTarget.IsA('SeaPawn'))
            {
                if (_EditorModeParam._FocusPartType != 0)
                    _EditorModeParam._FocusPartIndex++;
                //else
                //{
                //    Ship = SeaPawn(_EditorModeParam._TraceTarget);
                //    PlayerData = Ship.GetClientPlayerData();
                //    if (PlayerData)
                //}
            }
            else if (_EditorModeParam._TraceTarget.IsA('SeaAircraft'))
            {
                Aircraft = SeaAircraft(_EditorModeParam._TraceTarget);
                idx = `ClientWorldInfo._AircraftList.Find(Aircraft._PendingData);
                if (idx == -1)
                {
                    foreach `ClientWorldInfo._AircraftList(AircraftData)
                    {
                        if (AircraftData._Instance == none)
                            continue;

                        _EditorModeParam._TraceTarget = AircraftData._Instance;
                        break;
                    }
                }
                else
                {
                    idx = (idx + 1) % `ClientWorldInfo._AircraftList.Length;
                    AircraftData = `ClientWorldInfo._AircraftList[idx];
                    while (AircraftData != Aircraft._PendingData)
                    {
                        if (AircraftData._Instance != none)
                        {
                            _EditorModeParam._TraceTarget = AircraftData._Instance;
                            break;
                        }

                        idx = (idx + 1) % `ClientWorldInfo._AircraftList.Length;
                        AircraftData = `ClientWorldInfo._AircraftList[idx];
                    }
                }
            }
        }
    }
}

native function PlayShakeAnim(ECameraShakeLevel CameraShakeLevel);

simulated function UpdateCameraLensEffects( const out TViewTarget OutVT )
{
	local int Idx;

	for (Idx=0; Idx<CameraLensEffects.length; ++Idx)
	{
		if (CameraLensEffects[Idx] != None)
		{
			CameraLensEffects[Idx].UpdateLocation(OutVT.POV.Location, OutVT.POV.Rotation, OutVT.POV.FOV);
		}
	}
}

function PlayEagleEyeHurtCamAnim()
{
	PlayCameraAnim(_EagleEyeHurt);
}

function SwitchJetViewSound(bool Enable)
{
	if (_JetViewSoundCue == none)
		_JetViewSoundCue = SoundCue(DynamicLoadObject(_JetViewSoundURL, class'SoundCue'));

	if (Enable)
	{
		if (_JetViewSound == none && _JetViewSoundCue != none)
			_JetViewSound = CreateAudioComponent(_JetViewSoundCue, true, true);
	}
	else
	{
		if (_JetViewSound != none && _JetViewSound.IsPlaying())
		{
			_JetViewSound.Stop();
			_JetViewSound = none;
		}
	}
}

native function bool LineExtentBoundIntersection(out vector OutHitBound, out float HitTime, vector InStartPos, vector InEndPos, vector2D InMinBound, vector2D InMaxBound, vector2D InExtent);

//  当前是否正在播放镜头动画
native function bool HasActiveCameraAnim();

/////////////////////// DEBUG INTERFACE
simulated function GetCameraDebug( out Array<String> DebugInfo )
{
	local vector		Loc;
	local rotator		Rot;
	ViewTarget.Target.GetActorEyesViewPoint(Loc, Rot);
	DebugInfo[DebugInfo.Length] = "----Camera----: ";
	DebugInfo[DebugInfo.Length] = "ActorEyeViewPoint Loc: "$Loc;
	DebugInfo[DebugInfo.Length] = "ActorEyeViewPoint Rot: "$Rot;
}

simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	local Array<String>	DebugInfo;
	local int			i;

	super.DisplayDebug(HUD, out_YL, out_YPOS);

	GetCameraDebug( DebugInfo );

	Hud.Canvas.SetDrawColor(128,200,0);
	for (i=0;i<DebugInfo.Length;i++)
	{
		Hud.Canvas.DrawText( "  " @ DebugInfo[i] );
		out_YPos += out_YL;
		Hud.Canvas.SetPos(4, out_YPos);
	}
}

state ProjectileView
{
	function BeginState(name PreviousStateName)
	{
        local vector Loc;
        local vector LookAt;
        local SeaUIFightController ui;

		_PrevCameraStyle = PreviousStateName;
		CameraStyle = 'ProjectileView';

        global.BeginState(PreviousStateName);

        ViewTarget.POV.FOV = _ProjectileViewParam._FOV;

        ViewTarget.Target.GetActorEyesViewPoint(Loc, ViewTarget.POV.Rotation);
        ViewTarget.POV.Rotation.Pitch = _ProjectileViewParam._InitPitch * DegToUnrRot;
        LookAt = vect(1,0,0) >> ViewTarget.POV.Rotation;
        _ProjectileViewParam._LastTracePos = Loc + LookAt * _ProjectileViewParam._Distance;

        PCOwner.SetRotation(ViewTarget.POV.Rotation);

        ui = SeaUIFightController(`SeaUIController);
        if (ui != none)
            ui._CrossHair.Hide();
	}

    function EndState(name NextStateName)
	{
        local SeaUIFightController ui;

        _ProjectileViewParam._TraceTarget = none;
		Super.EndState(NextStateName);

        ui = SeaUIFightController(`SeaUIController);
        if (ui != none)
            ui._CrossHair.Show();
	} 

	function UpdateVT(out TViewTarget OutVT, float DeltaTime)
	{
        local vector LookAt, Right, CamUp, Loc;
        local SeaPlayerControllerFight Player;

        if (_ProjectileViewParam._TraceTarget == none)
            return;

        Player = SeaPlayerControllerFight(PCOwner);
        if (Player == none)
            return;

		super.UpdateVT(OutVT, DeltaTime);

        OutVT.Target.GetActorEyesViewPoint(Loc, OutVT.POV.Rotation);
        GetAxes(OutVT.POV.Rotation, LookAt, Right, CamUp);

        LookAt = vect(1,0,0) >> OutVT.POV.Rotation;
        _ProjectileViewParam._LastTracePos = VInterpTo(_ProjectileViewParam._LastTracePos
                                                    , _ProjectileViewParam._TraceTarget.Location
                                                    , DeltaTime
                                                    , _ProjectileViewParam._InterpSpeed);
        OutVT.POV.Location = _ProjectileViewParam._LastTracePos - LookAt * _ProjectileViewParam._Distance;

        PCOwner.SetRotation(OutVT.POV.Rotation);
	}
}

///< 观察者模式左键SHIFT视角
state ObserverView extends FightJetView
{

}

defaultproperties
{
	_ShakeCameraAnim[0]=CameraAnim'Common_Material_Template.CameraAnim.CameraAnim_LittleHurt'
	_ShakeCameraAnim[1]=CameraAnim'Common_Material_Template.CameraAnim.CameraAnim_MiddleHurt'
	_ShakeCameraAnim[2]=CameraAnim'Common_Material_Template.CameraAnim.CameraAnim_LargeHurt'
	_ShakeCameraAnim[3]=CameraAnim'Common_Material_Template.CameraAnim.CameraAnim_Fire01'
	_ShakeCameraAnim[4]=CameraAnim'Common_Material_Template.CameraAnim.CameraAnim_Fire02'
	_EagleEyeHurt=CameraAnim'Common_Material_Template.CameraAnim.CameraAnim_EagleEyes'

	_bEnableBlend=true
    _RadialMoveFlag=0
    _SideMoveFlag=0
    _VerticalMoveFlag=0
    _FOVZoomFlag=0
	RainState=0
	bHidden=false
}
