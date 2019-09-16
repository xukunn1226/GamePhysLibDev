//------------------------------------------------------------------------
//	@brief	���ػ�������������ʵ��
//	@author	chenpu
//	@date	2013-6-8
//------------------------------------------------------------------------
//	@brief	Ѳ���ٶ�
//DECLARE_PROPERTY_ACCESSOR(float, CruiseSpeed)
void PhysAircraft::SetCruiseSpeed(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_Speed = _value;
}

float PhysAircraft::GetCruiseSpeed()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 0.f;

	return pData->_Speed;
}

//	@brief	��鷶Χ
//DECLARE_PROPERTY_ACCESSOR(float, SensorRange)
void PhysAircraft::SetSensorRange(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_SensorRange = _value;
}

float PhysAircraft::GetSensorRange()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 0.f;

	return pData->_SensorRange;
}

//	@brief	�ڵ�����
//DECLARE_PROPERTY_ACCESSOR(float, TrajectoryGravity)
void PhysAircraft::SetTrajectoryGravity(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_Gravity = _value;
}

float PhysAircraft::GetTrajectoryGravity()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 0.f;

	return pData->_Gravity;
}

//	@brief	����б��
//DECLARE_PROPERTY_ACCESSOR(float, AscendSlope)
void PhysAircraft::SetAscendSlope(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_AscendSlope = _value;
}

float PhysAircraft::GetAscendSlope()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 1.f;

	return pData->_AscendSlope;
}

//	@brief	����б��
//DECLARE_PROPERTY_ACCESSOR(float, SwoopSlope)
void PhysAircraft::SetSwoopSlope(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_SwoopSlope = _value;
}

float PhysAircraft::GetSwoopSlope()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 1.f;

	return pData->_SwoopSlope;
}

//	@brief	��Ӧʱ��
//DECLARE_PROPERTY_ACCESSOR(float, EchoInterval)
void PhysAircraft::SetEchoInterval(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_EchoInterval = _value;
}

float PhysAircraft::GetEchoInterval()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 0.f;

	return pData->_EchoInterval;
}

//	@brief	��������ʱ��
//DECLARE_PROPERTY_ACCESSOR(float, ExLowFlyingTime)
void PhysAircraft::SetExLowFlyingTime(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_ExLowFlyingTime = _value;
}

float PhysAircraft::GetExLowFlyingTime()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 0.f;

	return pData->_ExLowFlyingTime;
}

//	@brief	�����ٶ�
//DECLARE_PROPERTY_ACCESSOR(float, LayerChangeSpeed)
void PhysAircraft::SetLayerChangeSpeed(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_LayerChangeSpeed = _value;
}

float PhysAircraft::GetLayerChangeSpeed()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 0.f;

	return pData->_LayerChangeSpeed;
}

//	@brief	��ʼͶ������
//DECLARE_PROPERTY_ACCESSOR(float, InitPrecision)
void PhysAircraft::SetInitPrecision(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_InitPrecision = _value;
}

float PhysAircraft::GetInitPrecision()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 0.f;

	return pData->_InitPrecision;
}

//	@brief	��׼Ͷ������
//DECLARE_PROPERTY_ACCESSOR(float, Precision)
void PhysAircraft::SetPrecision(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_Precision = _value;
}

float PhysAircraft::GetPrecision()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 0.f;

	return pData->_Precision;
}

//	@brief	Ͷ������ϵ��
//DECLARE_PROPERTY_ACCESSOR(float, PrecisionFactor)
void PhysAircraft::SetPrecisionFactor(float _value)
{
	_PrecisionFactor = _value;
}

float PhysAircraft::GetPrecisionFactor()
{
	return _PrecisionFactor;
}

//	@brief	��Ȧ�ٶ�
//DECLARE_PROPERTY_ACCESSOR(float, Collimation)
void PhysAircraft::SetCollimation(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_Collimation = _value;
}

float PhysAircraft::GetCollimation()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 0.f;

	return pData->_Collimation;
}

//	@brief	ת�򾫶�Ӱ��
//DECLARE_PROPERTY_ACCESSOR(float, TurningAffect)
void PhysAircraft::SetTurningAffect(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_TurningAffect = _value;
}

float PhysAircraft::GetTurningAffect()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 0.f;

	return pData->_TurningAffect;
}

//	@brief	���㾫��Ӱ��
//DECLARE_PROPERTY_ACCESSOR(float, LayerAffect)
void PhysAircraft::SetLayerAffect(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_TurningAffect = _value;
}

float PhysAircraft::GetLayerAffect()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 0.f;

	return pData->_TurningAffect;
}

//	@brief	Ŀ��㼶
//DECLARE_PROPERTY_ACCESSOR(FlightLayer, PendingLayer)
void PhysAircraft::SetPendingLayer(FlightLayer _value)
{
	if (_value < LayerNum)
		_PendingLayer = _value;
}

FlightLayer PhysAircraft::GetPendingLayer()
{
	return _PendingLayer;
}

//	@brief	������ٶ�
//DECLARE_PROPERTY_ACCESSOR(float, DiveAcceleration)
void PhysAircraft::SetDiveAcceleration(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_DiveAcceleration = _value;
}

float PhysAircraft::GetDiveAcceleration()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 0.f;

	return pData->_DiveAcceleration;
}

//	@brief	�������ٶ�
//DECLARE_PROPERTY_ACCESSOR(float, PullAcceleration)
void PhysAircraft::SetPullAcceleration(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_PullAcceleration = _value;
}

float PhysAircraft::GetPullAcceleration()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 0.f;

	return pData->_PullAcceleration;
}

//	@brief	Ͷ��ɢ��������
//DECLARE_PROPERTY_ACCESSOR(float, DiffuseEccentricity)
void PhysAircraft::SetDiffuseEccentricity(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_DiffuseEccentricity = _value;
}

float PhysAircraft::GetDiffuseEccentricity()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 1.f;

	return pData->_DiffuseEccentricity;
}

//	@brief	ս�����ɹ�������
//DECLARE_PROPERTY_ACCESSOR(float, AttackExtent)
void PhysAircraft::SetAttackExtent(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_AttackExtent = _value;
}

float PhysAircraft::GetAttackExtent()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 0.f;

	return pData->_AttackExtent;
}

//	@brief	ս�����ɹ����Ƕ�
//DECLARE_PROPERTY_ACCESSOR(float, AttackScope)
void PhysAircraft::SetAttackScope(float _value)
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return;

	pData->_AttackScope = _value;
}

float PhysAircraft::GetAttackScope()
{
	AircraftData* pData = DynamicCast(AircraftData, _ComponentData);
	if ( !pData )
		return 0.f;

	return pData->_AttackScope;
}