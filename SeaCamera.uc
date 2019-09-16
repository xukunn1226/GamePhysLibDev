class SeaCamera extends Camera
	config(Game)
	dependson(SeaPlayerControllerFight)
	native;

//����trace,���ֵ�����㹻��Ϳ���
const MAX_TRACE_RADIUS = 90000000;


//add by yeleiyu@icee.cn for �ѻ��ӽ�
var PostProcessChain TelescopeViewPPC;
//add by yeleiyu@icee.cn for �ѻ��ӽ�
var array<PostProcessChain> OldPostProcessChain;

//add by yeleiyu@icee.cn for ���깦��
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




//Ŀǰ����֧�ֵ��ӽ�����������һ��bitλ��ʽ��ֵ��ǣ��߻����ñ���ʹ��
const FirstPerson	= 1;
const ThirdPerson	= 2;
const Telescope		= 4;
const ArtilleryEye	= 8;
const FightJetEye	= 16;
const BirdEye		= 32;
const FreeCamera	= 64;

/** ��һ������ӽ� */
var name _PrevCameraStyle;

var globalconfig float		_ToleranceDistance;
/** ��������л�ģʽ����������е��˵�ǰ�ӽǲ�֧�ֵ�������Զ���������б�Ѱ�Ҹ����ȼ��Ƽ��ӽ�*/
var globalconfig array<INT> _PriorityCamSwitchMode;

// ӥ���ӽǵ���Listener��λ��
var globalconfig float _ListenerHeightScale;
var globalconfig float _ListenerHeightOffset;

// Ŀ����������
var SeaPawn						_LockedPawn;
var int							_LockedDeltaYaw;

var bool						_bEnableBlend;
var TPOV						_BlendedStartPOV;			// ������ʼPOV
var ViewTargetTransitionParams	_BlendParamsWhenSwitch;
var ViewTargetTransitionParams	_BlendParamsWhenFollow;
var float						_BlendTimeWhenSwitch;
var float						_BlendTimeWhenFollow;
var bool						_bAlwaysLookAt;				// camera�л�ʱ�Ƿ���Ҫʼ�տ���ĳ��
var vector						_PointLookAt;				// LookAt�㣬_bAlwaysLookAt==trueʱ��Ч

var vector2D					_MouseDelta;				// mouse offset
var vector2D					_MouseDeltaByDrag;
var vector2D					_MinBound;					// ��С�߽�ֵ
var vector2D					_MaxBound;					// ���߽�ֵ
var	float						_MaxSceneHeight;			// �������߶�
var float						_OceanZ;					// ��ƽ��߶�ֵ

var bool						_bUseSpecifiedLookAtPoint;	// �ӽ��л�ʱʹ��ָ��LookPoint
var vector						_SpecifiedLookAtPoint;		// ָ��LookAtPoint
var bool						_bUseSpecifiedLookAtPointWhenFollow;	// ״̬�ڹ���ʱ����ָ����
var vector						_SpecifiedLookAtPointWhenFollow;

////// ��һ�˳��ӽǲ���
struct native FirstViewParam
{
	var float		_CustomizedFirstFOV;			// ��һ�˳�FOV
	var float		_FirstPersonViewOffset;			// ��һ�˳��ӽ�POV���߱���
	var ViewTargetTransitionParams	_BlendParams;	// �л�����һ�˳��ӽ�ʱ�Ĺ��ɲ���
};
var globalconfig FirstViewParam		_FirstViewParam;


////// �����˳��ӽǲ���
struct native FreeViewParam
{
	var float			_DefaultFOV;
	var float			_Ratio;						// �̰��᳤�볤���᳤����
	var array<float>    _ScaleExpandLongAxle;		// ��Բ�����᳤�������õĳ����᳤�ȱ�ֵ
    var float		    _ViewPosHeightRatio;        // �۲��߶ȱ���
	var float			_LimitPitchMax;				// �ӽ�PITCH���ƣ� in degree
	var float			_LimitPitchMin;				// �ӽ�PITCH���ƣ� in degree
	var float			_FixLength;                 // �̶����᳤�ȣ������Ϊ0���͸������ֵ������ʵ�ʴ��������㳤��
	var float			_RejectParam;				// Camera����������Ŀǰ�������ڵ����˳�ʹ��
	var ViewTargetTransitionParams	_BlendParams;	// �л��������˳��ӽ�ʱ�Ĺ��ɲ���
	var ViewTargetTransitionParams	_InternalBlendParams;	// ״̬���л���λʱ�Ĺ��ɲ���

	var array<float>	_ScaleLevelList;			// ÿ�����������ı���
	var	float			_BasicHalfLongAxle;			// ���������᳤��	_BasicHalfLongAxle = 0.5f * Length * _ScaleExpandLongAxle;
	var	float			_BasicHalfShortAxle;		// �����̰��᳤��	_BasicHalfShortAxle = _BasicHalfLongAxle * _Ratio;
	var int				_CurAxleLevel;              // ��ǰʹ����һ����_ScaleExpandLongAxle
	var float			_CurScaleAlpha;
	var float			_StartScaleAlpha;			// 
	var float			_PendingScaleAlpha;
	var float			_ScaleAlpha;				// ��λ����ʱ�Ĳ���,[0, 1]
};
var globalconfig FreeViewParam	_FreeViewParam;		// ս���ڵ����˳��ӽǲ���
var globalconfig FreeViewParam	_FreeViewParamEx;	// �����۲콢��ʱ�����˳��ӽǲ���



/////// ��Զ���ӽǲ���
struct native TelescopeViewParam
{
	var array<float>				_TelescopeViewOffset;				// �Ѿ��ӽ�POV���߱���
	var array<float>				_TelescopeViewOffsetExceptCannon;	// �ǻ�������ʱ�Ѿ��ӽ�POV���߱���
	var array<float>				_TelescopeZoomLevel;				// ÿ������fovֵ
	var ViewTargetTransitionParams	_BlendParams;						// �л�����Զ���ӽ�ʱ�Ĺ��ɲ���
	var ViewTargetTransitionParams	_InternalBlendParams;				// ״̬���л���λʱ�Ĺ��ɲ���

	var bool						_bUseCannon;
	var int							_CurrTelescopeLevel;				// ��ǰ��Զ�����ŵȼ�
	var float						_CurScaleAlpha;
	var float						_CurrViewOffset;
};
var globalconfig TelescopeViewParam _TelescopeViewParam;



////// �����ӽǲ���
struct native ArtilleryViewParam
{
	var float		_DefaultFOV;
	var Rotator		_DefaultCamRotation;			// ���Ĭ��ת��
	var float		_MaxZoomOut;					// �������
	var float		_MinZoomOut;					// ��С����
	var float		_DefaultZoomOut;				// Ĭ������
	var float		_MaxZoomOutCached;				// ������Ż��棬�����ػ�ӥ���ӽ�ʹ��
	var float		_MinZoomOutCached;				// ��С���Ż��棬�����ػ�ӥ���ӽ�ʹ��
	var float		_DefaultZoomOutCached;			// Ĭ�����Ż��棬�����ػ�ӥ���ӽ�ʹ��
	var float		_CurrentZoomOut;
	var float		_EagleEyeMoveSpeed;				// ӥ���ӽ������ƽ���ٶ�
	var float		_EagleEyeZoomSpeed;             // ӥ���ӽ�����������ٶ�
	var ViewTargetTransitionParams	_BlendParams;	// �л��������ӽ�ʱ�Ĺ��ɲ���
	var ViewTargetTransitionParams	_InternalBlendParams;	// SetViewTargetPosʱ�Ĺ��ɲ���
	var ViewTargetTransitionParams	_InternalBlendParams2;	// ZOOM IN/ZOOM OUTʱ�Ĺ��ɲ���
};
var globalconfig ArtilleryViewParam _ArtilleryViewParam;     //����ӥ���ӽ��������
var globalconfig ArtilleryViewParam _FightJetViewParam;      //���ػ�ӥ���ӽ�������� ���ݲ߻����󣬽��ػ������ӥ���ӽ�ֻ�����ò����ϵĲ���
var globalconfig ArtilleryViewParam _ObserverViewParam;		 //�۲���ӥ���ӽ��������



////// ���ģʽ
struct native BirdViewParam
{
	var float			_MinAngle;					// ��ˮƽ�����С�н�
	var float			_MaxAngle;					// ��ˮƽ������н�
	var float			_MaxHeight;					// camera����߸߶ȣ�����ռ䣩
	var float			_ScaleOfAscend;				// ����ʱ�����ʣ�����aMouseY
	var float			_ScaleOfBeyond;				// ������߸߶Ⱥ����ǰ��ʱ�����ʣ�����aMouseY
	var array<float>	_BirdZoomLevel;				// ÿ������fovֵ
	var int				_CurrBirdLevel;				// ��ǰ��Զ�����ŵȼ�
	var ViewTargetTransitionParams	_BlendParams;	// �л������ģʽʱ�Ĺ��ɲ���
	var ViewTargetTransitionParams	_InternalBlendParams;	// ZOOM IN/ZOOM OUTʱ�Ĺ��ɲ���
	var ViewTargetTransitionParams	_InternalBlendParams2;	// SetViewTargetPos3Dʱ�Ĺ��ɲ���

	var float			_DistanceXY1;				// camera��λ������ˮƽ��н�Ϊ_MinAngleʱ�����camera��XYƽ��ľ���
	var float			_DistanceXY2;				// camera��������߸߶�����ˮƽ��н�Ϊ_MinAngleʱ�����camera��XYƽ��ľ���
	var float			_DistanceXY3;				// ��Զ������camera��ʼλ�õľ���
	var int				_PointInWhere;				// 1��[0, _BirdViewParam._DistanceXY1]��
													// 2��[_BirdViewParam._DistanceXY1, _BirdViewParam._DistanceXY2]
													// 3, [_BirdViewParam._DistanceXY2, _BirdViewParam._DistanceXY3]
	var float			_DistanceXY;				// LookAt����ViewTarget.Location��XYƽ��ľ���
};
var globalconfig BirdViewParam		_BirdViewParam;

////// �༭ģʽ
struct native EditorModeParam
{
	var float		_DefaultFOV;                    //  Ĭ��FOV
    var float       _MinFOV;                        //  ��СFOV
    var float       _MaxFOV;                        //  ���FOV
    var int         _TranslateRatio;                //  �ƶ�ת������
    var int         _MinTransRatio;                 //  ��С�ƶ�ת������
    var int         _MaxTransRatio;                 //  ����ƶ�ת������
    var int         _RotateRatio;                   //  ��ת�ٶ�
    var int         _MinRotRatio;                   //  ��С��ת�ٶ�
    var int         _MaxRotRatio;                   //  �����ת�ٶ�
    var int         _ZoomRatio;                     //  FOV��������
    var bool        _AutoReverseFOV;                //  �Զ��ָ�Ĭ��FOV
    var float       _Distance;
    var int         _FocusPartType;
    var int         _FocusPartIndex;
    var Pawn        _TraceTarget;
};
var globalconfig EditorModeParam _EditorModeParam;

/** �༭��ģʽCamera�������*/
var int _RadialMoveFlag;
var int _SideMoveFlag;
var int _VerticalMoveFlag;
var int _FOVZoomFlag;

////// �ڵ�׷���ӽ�
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
	/** ��΢*/
	ECSL_LITTLE,
	/** �е�*/
	ECSL_MIDDLE,
	/** ����*/
	ECSL_LARGE,
    /** ���������*/
    ECSL_FIRE_EXTREME,
    /** �������С*/
    ECSL_FIRE_NORMAL
};
/** ����𶯶���*/
var array<CameraAnim> _ShakeCameraAnim;
/** ӥ���ӽ��ܻ���ʾ*/
var CameraAnim _EagleEyeHurt;
/** �����𶯶���*/
var array<CameraAnim> _FireRecoilCameraAnim;
/** ���ػ����ͶӰ����Camera����Ч����*/
var globalconfig INT _SquadShadowValidDistance;

// ӥ���ӽ��µı�����Ч
var globalconfig String	_JetViewSoundURL;
var SoundCue	        _JetViewSoundCue;
var AudioComponent      _JetViewSound;

//add by yeleiyu@icee.cn for �ѻ��ӽ�
native function GetCurResolution();
native function InitCurResolution();

//add by yeleiyu@icee.cn for �ѻ��ӽ�
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
//add by yeleiyu@icee.cn for ���깦��
function InitializeFor(PlayerController PC)
{
	//add by yeleiyu@icee.cn for ���깦��
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

	// �޸�Ĭ�ϵ����˳�ʱ��FOV
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
	// ����Ŀ��λ��
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


	//add by yeleiyu@icee.cn for ���깦��
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

// ����CAMERA�������̺���
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
	_MouseDelta = SeaPlayerController(PCOwner)._MouseDelta;							// ��ȡ��굱ǰ֡��ƫ����
	_MouseDeltaByDrag = SeaPlayerController(PCOwner)._MouseDeltaByDrag;				// ��ȡ��굱ǰ֡��ƫ����
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
			_bAlwaysLookAt = true;			// ������������ʼ��LookAt������Ҫ����NextStateNameʱ��һ���ж�
		}
		else
		{
			_bAlwaysLookAt = false;
		}
	}
}

// ��ͬ״̬������CAMERAλ�õ�ƫ��
function ModifyCameraLoc(out vector InCameraLoc, float Height);

/**
 * ӥ���ӽ����ƶ�CAMERA��Target
 */
function SetViewTargetPos(Vector2D InLookAt);

/**
 * ��Ŀ�������camera
 */
function SetViewTargetPos3D(vector InLookAt);

/** �Ӿ��Զ/����
 * In/Out�¼���PC����
*/
function OnZoomIn(int iSpeed);
function OnZoomOut(int iSpeed);

/** SHIFT��Ӧ����
 * @param ActivePartType		��ǰ���������
 * @param ViewModeList			��ǰ����֧�ֵ��ӽ�����
*/
function ToggleView(int ActivePartType, int ViewModeList);

function SwitchPartType(SwitchPartFeedback Feedback);

function SetViewLockParam(SeaPawn LockedPawn, int LockedDeltaYaw)
{
	_LockedPawn = LockedPawn;
	_LockedDeltaYaw = LockedDeltaYaw;
}

/** ��ǰ���Ŀ���Ƿ����Լ�
 */
function bool IsWatchingSelf()
{
	return (ViewTarget.Target == PCOwner.Pawn);
}

/** �л��۲���
 * @param NewViewTarget Ŀ��۲���
 * @param TransitionParams �л��۲���ʱ��ͷ���ֵ�����,�絭�뵭��Ч��
 */
function SwitchViewTarget(Actor NewViewTarget, optional ViewTargetTransitionParams TransitionParams)
{
	local ViewTargetTransitionParams NullParams;

	// �л��۲���ʱǿ�Ƶ������˳�
	SetViewTarget(NewViewTarget, NullParams);		// ���л�ViewTarget

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

// �ӽ�ģʽö��ֵ��߻����õ��ӽ�ֵת���ӿ�
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

// �߻����õ��ӽ�ģʽ��camera styleת��
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

/** �������ģʽ��Ψһ�ӿڣ����ǵ�ǰ����ѡ������Ƶ��ӽ�
 *  @param NewCameraMode
 *  @param bForceSwitch		�Ƿ��������������ƶ�ǿ���л��ӽ�
 *  @param bSwitchStateEvenTheSameStyle		TRUE����ʹcamerastyle�������л�style��false����֮
 *											��style���䣬��ViewTarget�����仯ʱ��Ҫ�л�
 *	@param bUseSpecifiedLookAtPoint			�л���NextStateʱʹ��ָ����ΪLookAtPoint
 *	@param SpecifiedLookAtPoint				ָ����LookAtPoint��bUseSpecifiedLookAtPoint==true��������
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

	// �����ӽǺʹ��������˳��ӽ����ȴ���
	if (NewCameraMode == 'FreeCameraView' || NewCameraMode == 'ThirdPersonViewEx')
    {
		GotoState(NewCameraMode);
        return true;
    }

	if( !bForceSwitch )
	{ // ��������������
		ViewModeList = SeaPlayerControllerFight(PCOwner).GetViewModeListByActiveType();
		if(ViewmodeList == 0)
		{
			`UClog(FUNC_TAG_None,"None viewmode list supported.");
			return false;
		}

		// ����ǰѡ����������Ͳ�֧�ִ��ӽ�ģʽ��EMode������������ȼ�ѡ��һ�����ʺϵ��ӽ�ģʽ
		if( (ViewmodeList & TranslateViewFlag(NewCameraMode)) == 0)
		{
			foreach _PriorityCamSwitchMode(PrioMode)
			{
				if((ViewmodeList & PrioMode) != 0)
				{
					NewCameraMode = TranslateCameraStyle(PrioMode);

					// �������NewCameraMode�����ı䣬�����NewCameraMode����ѡ����ɲ���
					TransitionParams = GetTransitionParamsByCameraStyle(NewCameraMode);
					break;
				}
			}
		}
	}

	if( CameraStyle != NewCameraMode || bSwitchStateEvenTheSameStyle )
	{
		// ֪ͨ������ӽ�ģʽ
		`SeaNetClient.ViewModeSelectPost(TranslateViewFlag(NewCameraMode));
	
		_bUseSpecifiedLookAtPoint = bUseSpecifiedLookAtPoint;
		_SpecifiedLookAtPoint = SpecifiedLookAtPoint;

		GotoState(NewCameraMode);

		_bUseSpecifiedLookAtPoint = false;		// ��ԭ��ʹ��Ĭ��ģʽ�л��ӽ�

		BeginCameraSwitch(TransitionParams);

		// PVE���ֽ̳��¼�
		Param.SS.AddItem(string(NewCameraMode));
		`ClientWorldInfo.InputPVEEvent(EPVETE_SwitchViewMode, Param);
	}

	return true;
}

// camera�ڲ�ͬģʽ֮���л��Ŀ�ʼ/����
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
	_BlendTimeWhenFollow = 0.f;			// ���follow time��Switch�׶β�����follow����
	_bAlwaysLookAt = false;
	OnEndCameraSwitch();
}

// camera��ģʽ�ڸ���״̬�Ŀ�ʼ/�����ӿ�
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

/** ��ӥ���ӽ��л�Ϊ�����ӽ�ʱ,Ҫ�趨һ������ľ�ͷ����
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

// �����ӽǣ�ʹ�ù۲��߳��򣨺���ROLL,PITCH��
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

	// ������Ŀ��ʱ
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
		
		// ǰһ״̬�ж�Ϊ_bAlwaysLookAt������Ҫ��һ���ж�
		if( _bAlwaysLookAt )
		{
			ViewTarget.Target.GetActorEyesViewPoint(ViewLoc, ViewRot);
			StartPos = ViewLoc;
			EndPos = StartPos + Normal(Vector(ViewRot)) * MAX_TRACE_RADIUS;
			_bAlwaysLookAt = false;
			if( SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0.f, HitLoc) )
			{
				// ǰ�������ж��ݲ���һ����Χ����Ϊ��ͬ
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
		// ��ǰ������֧��ӥ���ӽ������ȣ�����ѻ�
		// ����״̬�޿����л�����һ�˳ƣ��ʲ�����
		if( ActivePartType == -1 && (ViewModeList & TranslateViewFlag('FightJetView')) != 0 )
		{ // ѡ�н��ػ�ʱ
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
	//add by yeleiyu@icee.cn for �ѻ��ӽ�
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
		//add by yeleiyu@icee.cn for �ѻ��ӽ�
		//�ѻ��ӽ�
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

	//add by yeleiyu@icee.cn for �ѻ��ӽ�
	function DoChangeView()
	{
		local SeaPlayerControllerFight PC;
		local SeaUIFightController UI;
		local LocalPlayer LP;
		//add by yeleiyu@icee.cn for �ѻ��ӽ�
		//�ѻ��ӽ�
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

		//add by yeleiyu@icee.cn for �ѻ��ӽ�
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
			// �ӵ�һ�˳��л���ʹ�õ�һ��
			SetZoomLevel(0);
		}
		else
		{
			// ����ѻ�ģʽĬ��ʹ���ϴ�����
		    SetZoomLevel(_TelescopeViewParam._CurrTelescopeLevel);
		}

        LookAtTarget();
		
		//if( PreviousStateName == 'ThirdPersonView' || PreviousStateName == 'FirstPersonView')
		//	_bAlwaysLookAt = false;
		
		// ǰһ״̬�ж�Ϊ_bAlwaysLookAt������Ҫ��һ���ж�
		if( _bAlwaysLookAt )
		{
			ViewTarget.Target.GetActorEyesViewPoint(ViewLoc, ViewRot);
			StartPos = ViewLoc;
			EndPos = StartPos + Normal(Vector(ViewRot)) * MAX_TRACE_RADIUS;
			_bAlwaysLookAt = false;
			if( SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0.f, HitLoc) )
			{
				// ǰ�������ж��ݲ���һ����Χ����Ϊ��ͬ
				if( Abs(HitLoc.X - _PointLookAt.X) < _ToleranceDistance && Abs(HitLoc.Y - _PointLookAt.Y) < _ToleranceDistance )
				{
					_bAlwaysLookAt = true;
					_PointLookAt = HitLoc;
				}
			}
		}

		// PVP���ֽ̳��¼�
		`ClientWorldInfo.InputPVPEvent(EPVPTE_SnipeView);
	}

	//add by yeleiyu@icee.cn for �ѻ��ӽ�
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
		
		// ����ʰ������Zֵ
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

	//add by yeleiyu@icee.cn for �ѻ��ӽ�
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

	/** ������Զ�����ŵȼ�
	 *  @param iLevel ����FOV�ȼ�
	 *  @return �������ú�FOV�ȼ�
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

		// ��camera�Դ���Switch�׶λ���follow time���������µ�_PendingScaleAlpha
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
		{ // ��ǰ�Ѵ��ڵ�һ��FOV
			SetCamMode('FirstPersonView',,_FirstViewParam._BlendParams);
		}
		else
		{
			SetZoomLevel(_TelescopeViewParam._CurrTelescopeLevel-1);
		}
	}

	// �л�����
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

		// �л���֮ǰ���ӽ�ģʽ
		if( ActivePartType != -1 )
		{
			// ֧��ӥ��ģʽ�������л�
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
		{ // �����ӽ��л�ʱǿ��ʹ�ò�����LookAtPoint���㷨
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
		
		// ǰһ״̬�ж�Ϊ_bAlwaysLookAt������Ҫ��һ���ж�
		if( _bAlwaysLookAt )
		{
			ViewTarget.Target.GetActorEyesViewPoint(ViewLoc, ViewRot);
			StartPos = ViewLoc;
			EndPos = StartPos + Normal(Vector(ViewRot)) * MAX_TRACE_RADIUS;
			_bAlwaysLookAt = false;
			if(SeaPlayerControllerFight(PCOwner) != none && SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0.f, HitLoc) )
			{
				// ǰ�������ж��ݲ���һ����Χ����Ϊ��ͬ
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

		// ÿ�����ڻ���������ı�ֵ
		_FreeViewParam._ScaleLevelList.Length = 0;
		foreach _FreeViewParam._ScaleExpandLongAxle(Scale, Index)
		{
			_FreeViewParam._ScaleLevelList[Index] = Scale / _FreeViewParam._ScaleExpandLongAxle[_FreeViewParam._ScaleExpandLongAxle.Length - 1];
		}
				
		if( _PrevCameraStyle == 'BirdView' )
		{ // �����ģʽ�л���Ĭ�Ͻ���ڶ���
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

		// �������̰��᳤
		SeaPlayerControllerFight(PCOwner).GetPawnVolume(Length, Width, Height);
		if(_FreeViewParam._FixLength != 0)				// _FixLength��Ϊ0����ȡ���������õ�Length
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

		// ViewLoc���ӽ�LOOKAT��
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

	// ����camera����LOOKAT��Ϊ���ĵ���Բ�ռ��λ��
	// @BasicHalfLongAxle		��Բ�ռ䳤����
	// @BasicHalfShortAxle		��Բ�ռ�̰���
	// @TargetRot				��������
	// @CameraRot				Camera����
	function vector UpdateFreeViewParam(float BasicHalfLongAxle, float BasicHalfShortAxle,
										  rotator TargetRot, rotator CameraRot, int LimitPitch, float DeltaTime)
	{
		local matrix RotTM;
		local vector LocalDir;	// ��������ռ���ӽǳ���
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

		// �ӽ���Target����ռ�ĳ���
		Rot = CameraRot;
		Rot.Pitch = 0;
		Rot.Roll = 0;
        TargetRotation = TargetRot;
        TargetRotation.Pitch = 0;
        TargetRotation.Roll = 0;
		RotTM = MakeRotationMatrix(TargetRotation);		// ��������ռ�
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
				_FreeViewParam._StartScaleAlpha = _FreeViewParam._CurScaleAlpha;	// ��ǰscale alpha��Ϊ��ʼscale alpha
				_FreeViewParam._PendingScaleAlpha = _FreeViewParam._ScaleLevelList[_FreeViewParam._CurAxleLevel];
				_FreeViewParam._ScaleAlpha = 0.f;

				BeginCameraFollow(_FreeViewParam._InternalBlendParams);
			}

			// ��camera�Դ���Switch�׶λ���follow time���������µ�_PendingScaleAlpha
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
			_FreeViewParam._StartScaleAlpha = _FreeViewParam._CurScaleAlpha;	// ��ǰscale alpha��Ϊ��ʼscale alpha
			_FreeViewParam._PendingScaleAlpha = _FreeViewParam._ScaleLevelList[_FreeViewParam._CurAxleLevel];
			_FreeViewParam._ScaleAlpha = 0.f;

			BeginCameraFollow(_FreeViewParam._InternalBlendParams);
		}

		// ��camera�Դ���Switch�׶λ���follow time���������µ�_PendingScaleAlpha
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
		// ��ǰ������֧��ӥ���ӽ������ȣ�����ѻ�
		if(!PCOwner.IsInState('Dead'))
		{
			if( (ViewModeList & TranslateViewFlag('ArtilleryView')) != 0
				|| (ViewModeList & TranslateViewFlag('FightJetView')) != 0)
			{
				if( ActivePartType == -1 )
				{ // ѡ�н��ػ�ʱ
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
			// ����״̬����ӥ��ģʽ�л��������������������Ƿ�֧��
			SetCamMode('ArtilleryView', true, _ArtilleryViewParam._BlendParams);
		}
	}
}

// ����չʾ����ʱ�ĵ����˳��ӽ�
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

		// ÿ�����ڻ���������ı�ֵ
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
		
		// �������̰��᳤
		SeaPlayerControllerLobby(PCOwner).GetPawnVolume(Length, Width, Height);
		if(_FreeViewParamEx._FixLength != 0)				// _FixLength��Ϊ0����ȡ���������õ�Length
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

		// ViewLoc���ӽ�LOOKAT��
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
            //  ��������۲�����߼�⵽��ײ�У�������ں��ڣ�
            if (VSize(HitLocation - Pos) < _FreeViewParamEx._RejectParam)
                OutVT.POV.Location = HitLocation - ViewDir * _FreeViewParamEx._RejectParam;
            else
			    OutVT.POV.Location = Pos;
		}
		else if (SeaPlayerControllerLobby(PCOwner).CameraTraceEx(Pos - ViewDir * 100000, Pos, 0, HitLocation))
		{
            //  �������λ���⣬�����߷������ӳ����ϵ�ĳ���������λ�������߼�⣨������ں��ڣ�
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
			_FreeViewParamEx._StartScaleAlpha = _FreeViewParamEx._CurScaleAlpha;	// ��ǰscale alpha��Ϊ��ʼscale alpha
			_FreeViewParamEx._PendingScaleAlpha = _FreeViewParamEx._ScaleLevelList[_FreeViewParamEx._CurAxleLevel];
			_FreeViewParamEx._ScaleAlpha = 0.f;

			BeginCameraFollow(_FreeViewParamEx._InternalBlendParams);
		}

		// ��camera�Դ���Switch�׶λ���follow time���������µ�_PendingScaleAlpha
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
			_FreeViewParamEx._StartScaleAlpha = _FreeViewParamEx._CurScaleAlpha;	// ��ǰscale alpha��Ϊ��ʼscale alpha
			_FreeViewParamEx._PendingScaleAlpha = _FreeViewParamEx._ScaleLevelList[_FreeViewParamEx._CurAxleLevel];
			_FreeViewParamEx._ScaleAlpha = 0.f;

			BeginCameraFollow(_FreeViewParamEx._InternalBlendParams);
		}

		// ��camera�Դ���Switch�׶λ���follow time���������µ�_PendingScaleAlpha
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

		// ����ӥ���ӽ�������
		if (PCOwner.PlayerInput != none)
			PCOwner.PlayerInput.SetSensitivity(`CustomDataMgr._CtrlSettingsData._SpecialMouseSensitivity);

		// ����ʰ����ƫ����
		ListenerLocScale.X = 1.0;
		ListenerLocScale.Y = 1.0;
		ListenerLocScale.Z = _ListenerHeightScale;
		ListenerLocOffset.X = 0.0;
		ListenerLocOffset.Y = 0.0;
		ListenerLocOffset.Z = _ListenerHeightOffset;
		LocalPlayer(PCOwner.Player).SetListenerLocFactor(ListenerLocScale, ListenerLocOffset);

		// ӥ���ӽǱ�����Ч
		SwitchJetViewSound(true);

		if( !_bEnableBlend )
		{ // �޹���ʱ
			// ��ʾС��ͼ��������
			SeaUIFightController(`SeaUIController).ShowMiniMapViewArea(true);
		}
	}

	function EndState(name NextStateName)
	{
		local SeaUIFightController UIFight;

		// �ָ���ͨ�ӽ�������
		if (PCOwner.PlayerInput != none)
			PCOwner.PlayerInput.SetSensitivity(`CustomDataMgr._CtrlSettingsData._NormalMouseSensitivity);

		// �ָ�ʰ����ƫ����
		LocalPlayer(PCOwner.Player).ResetListenerLocFactor();

		UIFight = SeaUIFightController(`SeaUIController);
		if( UIFight != none )
		{
			// ����С��ͼ��������
			UIFight.ShowMiniMapViewArea(false);
		}

		SwitchJetViewSound(false);

		Super.EndState(NextStateName);
	}

	function OnEndCameraSwitch()
	{
		// ��ʾС��ͼ��������
		SeaUIFightController(`SeaUIController).ShowMiniMapViewArea(true);
	}
}

// ���������������Ŀ������
// ����ֵ��ʾ�����������ǰ�Ƿ���Ŀ��������
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
	// ����������ӽ����л���ӥ���ӽǣ���������һ֡��camera����
	// �ݲ�֧�ֳ�ʼΪӥ���ӽ�ģʽ
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

		// step 1. �����Чλ������
		ValidLocation = InitValidLocation();
		 
		// step 2. push data to ViewTarget
		InitViewTargetParam(ValidLocation);

		// ǰһ״̬�ж�Ϊ_bAlwaysLookAt������Ҫ��һ���ж�
		if( _bAlwaysLookAt )
		{
			StartPos = ViewTarget.POV.Location;
			EndPos = StartPos + Normal(Vector(ViewTarget.POV.Rotation)) * MAX_TRACE_RADIUS;
			_bAlwaysLookAt = false;
			if( SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0.f, HitLoc) )
			{
				// ǰ�������ж��ݲ���һ����Χ����Ϊ��ͬ
				if( Abs(HitLoc.X - _PointLookAt.X) < _ToleranceDistance && Abs(HitLoc.Y - _PointLookAt.Y) < _ToleranceDistance )
				{
					_bAlwaysLookAt = true;
					_PointLookAt = HitLoc;
				}
			}
		}

		// PVP���ֽ̳��¼�
		`ClientWorldInfo.InputPVPEvent(EPVPTE_EagleEyeView);
	}

	function EndState(name NextStateName)
	{
		Super.EndState(NextStateName);

        SeaPlayerControllerFight(PCOwner).UnlockTarget(false);
	}

	// ��úϷ�����ʼĿ���
	// @param StartPos	�۲�λ��
	// @param Dir		�۲쳯��
	// @param MinBound/MaxBound/MaxHeight	��С�߽�ֵ/���߽�ֵ/���߶�
	function vector GetValidLocation(vector StartPos, vector Dir, vector2D MinBound, vector2D MaxBound, float MaxHeight)
	{
		local vector EndPos;
		local vector HitLoc;
		local bool bTraceAnything;

		EndPos = StartPos + Dir * MAX_TRACE_RADIUS;
		bTraceAnything = SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0, HitLoc, true);

		// ��ⲻ������ʱʹ����ʼ�����
		ClampToBoundArea(HitLoc, bTraceAnything ? HitLoc : StartPos, MinBound, MaxBound, vect2d(-50, -50));

		// HitLoc�Ǵ������ӽǼ�⵽��Ŀ��㣬���ٴμ��
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

	// �����Чλ������
	function vector InitValidLocation()
	{
		local vector CameraLoc;
		local vector CameraDir;
		local vector ValidLocation;			// �л���ӥ�۵ĳ�ʼ�Ϸ�λ��
		
		CameraLoc = CameraCache.POV.Location;						// ��һ֡Cameraλ��
		CameraDir = Normal(Vector(CameraCache.POV.Rotation));		// ��һ֡Camera����
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
		
	// �л���ӥ���ӽ�
	// @param TargetPos		��TargetPosΪ����
	function SetViewTargetPos(Vector2D InLookAt)
	{
		local vector StartPos, EndPos;
		local vector HitLoc;
		
		StartPos.X = InLookAt.X;
		StartPos.Y = InLookAt.Y;
		StartPos.Z = _MaxSceneHeight;
		EndPos = StartPos + vect(0,0,-1) * MAX_TRACE_RADIUS;
					
		// �Ѳ���StartPos������������
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

		// step1. X����ƫ�Ƶ���ײ���
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

		// step2. Y����ƫ�Ƶ���ײ���
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

		// step3. Z����ƫ�Ƶ���ײ���
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

		// SetViewTargetPos������FOLLOW���ȼ����ߣ���ʱ������ZOOM IN/ZOOM OUT�Ĺ��ɲ���
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

		// SetViewTargetPos������FOLLOW���ȼ����ߣ���ʱ������ZOOM IN/ZOOM OUT�Ĺ��ɲ���
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
		
		// ����δ���Ƕ������ĳ�ʼֵ
		if( _FightJetViewParam._CurrentZoomOut == 0 )
		{
			_FightJetViewParam._CurrentZoomOut = _FightJetViewParam._DefaultZoomOut;
		}
		_FightJetViewParam._MinZoomOutCached = _FightJetViewParam._MinZoomOut;
		_FightJetViewParam._MaxZoomOutCached = _FightJetViewParam._MaxZoomOut;
		_FightJetViewParam._DefaultZoomOutCached = _FightJetViewParam._DefaultZoomOut;
		// ����MinZoomOut/MaxZoomOut,����ֵû����PITCH
		_FightJetViewParam._MinZoomOut = CalcRealDistance(_FightJetViewParam._MinZoomOut, _FightJetViewParam._DefaultCamRotation.Pitch);
		_FightJetViewParam._MaxZoomOut = CalcRealDistance(_FightJetViewParam._MaxZoomOut, _FightJetViewParam._DefaultCamRotation.Pitch);
		_FightJetViewParam._DefaultZoomOut = CalcRealDistance(_FightJetViewParam._DefaultZoomOut, _FightJetViewParam._DefaultCamRotation.Pitch);

		// step 1. �����Чλ������
		ValidLocation = InitValidLocation();

		// step 2. push data to ViewTarget
		InitViewTargetParam(ValidLocation);

		// ǰһ״̬�ж�Ϊ_bAlwaysLookAt������Ҫ��һ���ж�
		if( _bAlwaysLookAt )
		{
			StartPos = ViewTarget.POV.Location;
			EndPos = StartPos + Normal(Vector(ViewTarget.POV.Rotation)) * MAX_TRACE_RADIUS;
			_bAlwaysLookAt = false;
			if( SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0.f, HitLoc) )
			{
				// ǰ�������ж��ݲ���һ����Χ����Ϊ��ͬ
				if( Abs(HitLoc.X - _PointLookAt.X) < _ToleranceDistance && Abs(HitLoc.Y - _PointLookAt.Y) < _ToleranceDistance )
				{
					_bAlwaysLookAt = true;
					_PointLookAt = HitLoc;
				}
			}
		}

        //  ���뺽ĸӥ���ӽǣ�����Ѫ���ٶ�
        ui = SeaUIFightController(`SeaUIController);
        if (ui != none)
        {
            ui._CrossHair.Hide();
			ui._WeaponView._SwitchFireMode.SetVisible(false);
        }
		
		`SeaGameViewportClient.ShowCursor(true, true, true);
		`SeaGameViewportClient.SetScreenDragFlag(true); // ӥ���ӽ��£�������ק��Ļ
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

        //  �뿪��ĸӥ���ӽǣ���ʾѪ�����ٶ�
        ui = SeaUIFightController(`SeaUIController);
        if (ui != none && !PCOwner.IsInState('Dead') )
        {
			if( ship._ActivePartType == -1 )
			{ // ��ѡ���ػ�ʱ����׼��
				ui._CrossHair.HideCross();
			}
			else
			{
				ui._CrossHair.ShowCross();
			}
            ui._CrossHair.Show();
			ui._WeaponView._SwitchFireMode.SetVisible(true);
        }

		// ��ԭ��ʼֵ
		_FightJetViewParam._MinZoomOut = _FightJetViewParam._MinZoomOutCached;
		_FightJetViewParam._MaxZoomOut = _FightJetViewParam._MaxZoomOutCached;
		_FightJetViewParam._DefaultZoomOut = _FightJetViewParam._DefaultZoomOutCached;

		Super.EndState(NextStateName);
		
		`SeaGameViewportClient.ShowCursor(false);
		`SeaGameViewportClient.SetScreenDragFlag(false); // �뿪ӥ���ӽǣ���겻����ק��Ļ
	}

	// ��úϷ�����ʼĿ���
	// @param StartPos	�۲�λ��
	// @param Dir		�۲쳯��
	// @param MinBound/MaxBound/MaxHeight	��С�߽�ֵ/���߽�ֵ/���߶�
	function vector GetValidLocation(vector StartPos, vector Dir, vector2D MinBound, vector2D MaxBound, float MaxHeight)
	{
		local vector EndPos;
		local vector HitLoc;
		local bool bTraceAnything;

		EndPos = StartPos + Dir * MAX_TRACE_RADIUS;
		bTraceAnything = SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0, HitLoc, true);

		// ��ⲻ������ʱʹ����ʼ�����
		ClampToBoundArea(HitLoc, bTraceAnything ? HitLoc : StartPos, MinBound, MaxBound, vect2d(-50, -50));

		return HitLoc;
	}

	// �����Чλ������
	function vector InitValidLocation()
	{
		local vector ValidLocation;			// �л���ӥ�۵ĳ�ʼ�Ϸ�λ��
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

		// ����볡������ײ�㣬���Կ���ǽ
		if( !SeaPlayerControllerFight(PCOwner).TargetTrace(StartPos, EndPos, true, HitLoc) )
		{
			HitLoc = StartPos;
			HitLoc.Z = 0;
		}

		// ����ײ��������bound volume��
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
		
		// 1.ȷ��camera�����볡������ײ��
		StartPos = OutVT.POV.Location;
		SeaPlayerControllerFight(PCOwner).TargetTrace(StartPos, CameraDir, true, HitLocation);	// ��������ǽ
		
		// 2.�ƶ���ײ�㣨DeltaXY)��������BoundVolume��
		HitLocation.X += CameraDelta.X;
		HitLocation.Y += CameraDelta.Y;
		ClampToBoundArea(HitLocation, HitLocation, _MinBound, _MaxBound, vect2d(0,0));

		// 3.�����µ�λ���ٴμ���µ���ײ��
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
		
		// SetViewTargetPos������FOLLOW���ȼ����ߣ���ʱ������ZOOM IN/ZOOM OUT�Ĺ��ɲ���
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

		// SetViewTargetPos������FOLLOW���ȼ����ߣ���ʱ������ZOOM IN/ZOOM OUT�Ĺ��ɲ���
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


//////////////////////////// ���ģʽ
state BirdView
{
	// ���ģʽ�ĳ�ʼλ�ü��ѻ�ģʽʱ��λ��
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
		local float ZRelativeOcean;	// ��Ժ�ƽ��ĸ߶�
		local float UpperRange, LowerRange;
		local vector StartPos, EndPos;
		local vector LookDir;
		local vector LookAtPoint;

		_PrevCameraStyle = PreviousStateName;
		CameraStyle = 'BirdView';

        global.BeginState(PreviousStateName);

		// ��ʼ�����ģʽOriginalZ����Ժ�ƽ��ĸ߶�
		ViewTarget.Target.GetActorEyesViewPoint(TargetLoc, TargetRot);
		ZRelativeOcean = TargetLoc.Z - _OceanZ;

		// ������ò�������Ч��
		_BirdViewParam._MinAngle = Abs(_BirdViewParam._MinAngle) % 90.f;
		_BirdViewParam._MinAngle = FMax(1.f, _BirdViewParam._MinAngle);		// ����MinAngleΪ0
		_BirdViewParam._MaxAngle = FMax(_BirdViewParam._MinAngle, _BirdViewParam._MaxAngle);
		_BirdViewParam._MaxHeight = FMax(ZRelativeOcean + 100, _BirdViewParam._MaxHeight);
		_BirdViewParam._ScaleOfAscend = FMax(0.01f, _BirdViewParam._ScaleOfAscend);
		_BirdViewParam._ScaleOfBeyond = FMax(0.01f, _BirdViewParam._ScaleOfBeyond);
		_BirdViewParam._CurrBirdLevel = FMax(0, _BirdViewParam._CurrBirdLevel);

		// ����XY1,XY2,XY3����������֮�����Ч��
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
		// ���ݵ�ǰPOV�����һ֡ViewTarget.POV
		SetViewTargetPos3D(LookAtPoint);

		// ǰһ״̬�ж�Ϊ_bAlwaysLookAt������Ҫ��һ���ж�
		if( _bAlwaysLookAt )
		{
			StartPos = ViewTarget.POV.Location;
			EndPos = StartPos + Normal(Vector(ViewTarget.POV.Rotation)) * MAX_TRACE_RADIUS;
			_bAlwaysLookAt = false;
			if( SeaPlayerControllerFight(PCOwner).CameraTraceEx(StartPos, EndPos, 0.f, HitLoc) )
			{
				// ǰ�������ж��ݲ���һ����Χ����Ϊ��ͬ
				if( Abs(HitLoc.X - _PointLookAt.X) < _ToleranceDistance && Abs(HitLoc.Y - _PointLookAt.Y) < _ToleranceDistance )
				{
					_bAlwaysLookAt = true;
					_PointLookAt = HitLoc;
				}
			}
		}

		if( !_bEnableBlend )
		{ // �޹���ʱ
			// ��ʾС��ͼ��������
			SeaUIFightController(`SeaUIController).ShowMiniMapViewArea(true);
		}

		// PVP���ֽ̳��¼�
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

		ui.ShowMiniMapViewArea(true);           //  ��ʾС��ͼ��������
        ui._AimingView.SwitchAimingLine(true);  //  ��ʾ����
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
	
	// InLookAt,VertexA,VertexB1,VertexC1,VertexB2,VertexC2����ͬһƽ��
	// �����ڴ�������Ocean�Ľ���
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

		// �ж�InLookAt���ĸ�������,��Ocean�Ľ���
		if( PointInTriangle(InLookAt, VertexA, VertexB1, VertexC1) )
		{ // ��������1
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
			{ // ��������2
				_BirdViewParam._PointInWhere = 2;
			}
			else
			{ // ��������3
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
		{ // ��Ocean����������������
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
				// ����PointLookAtOcean��Bound��
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

		// ��������Զ�����
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
			// ���¼���_PointInWhere
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
		
		// �������������ζ���
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

		////////// step 1.	ȷ��Ŀ������ĸ�����
		InLookAt.Z += 0.01f;		// ��ֹInLookAt��CA������ļ������
		bLimitRange = !ClampToBoundArea(ClampToPos, InLookAt, _MinBound, _MaxBound, vect2d(-20.f, -20.f));		// InLookAt�ڻ��Χ����������Զ���
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

		// ȷ������VT.Rotation������ͬ����PC
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
			// �������������ζ���
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
		
			// ����Ƿ񳬳�Bound����
			PendingPointLookAtOcean = VertexA + LookDir * _BirdViewParam._DistanceXY;

			// ���¼���_PointInWhere
			PointLookAtOcean = DetermineWhichTriangle(PendingPointLookAtOcean, VertexA, VertexB1, VertexC1, VertexB2, VertexC2);
			
			OutVT.POV.Rotation = Rotator(PointLookAtOcean - TargetLoc);
			OutVT.POV.Location = TargetLoc;
		}
		else if( _BirdViewParam._PointInWhere == 2 )
		{
			// �������������ζ���
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

			// ����Ƿ񳬳�Bound����
			PendingPointLookAtOcean = VertexA + LookDir * _BirdViewParam._DistanceXY;

			// ���¼���_PointInWhere
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
			CameraRot.Yaw -= SeaPlayerControllerFight(PCOwner)._DeltaRot.Yaw;		// YAW���������ƣ���ԭ֮ǰֵ
			CameraRot.Pitch = 0;

			DeltaYaw = Atan(_MouseDelta.X * _BirdViewParam._ScaleOfBeyond / (_BirdViewParam._DistanceXY - _MouseDelta.Y * _BirdViewParam._ScaleOfBeyond)) * RadToUnrRot;

			CameraRot.Yaw += DeltaYaw;
			LookDir = Normal(Vector(CameraRot));


			// �������������ζ���
			VertexA = TargetLoc;
			VertexA.Z = _OceanZ;

			VertexB1 = TargetLoc;

			VertexC1 = VertexA + LookDir * _BirdViewParam._DistanceXY1;

			VertexB2 = TargetLoc;
			VertexB2.Z = _BirdViewParam._MaxHeight;

			VertexC2 = VertexA + LookDir * _BirdViewParam._DistanceXY2;
		
			// ����Ƿ񳬳�Bound����
			PendingPointLookAtOcean = VertexA + LookDir * _BirdViewParam._DistanceXY;

			// ���¼���_PointInWhere
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


/////////////////////////// ���������ģʽ�����༭��ģʽʹ��
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
            //  �Զ��ظ�FOV
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

//  ��ǰ�Ƿ����ڲ��ž�ͷ����
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

///< �۲���ģʽ���SHIFT�ӽ�
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
