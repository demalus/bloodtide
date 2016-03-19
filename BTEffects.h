//=============================================================
//
// Blood Tide, Version 1.0.0
// Copyright 2009-2010, by Team Blackfire
//
// This file is part of the game Blood Tide. It uses the
// C4 Engine from Terathon Software LLC. Their license
// can be viewed at:
//
// http://www.terathon.com/c4engine/license.php
//
// Our license agreement, however, is not so kind.
// You are free to use and modify this code, but you
// cannot sell it and you must give us credit if you use it.
// Violators will be fed to the Gorthax.
//
//=============================================================


#ifndef BTEffects_h
#define BTEffects_h

#include "C4Messages.h"
#include "C4Controller.h"
#include "MGEffects.h"
#include "BTControllers.h"

namespace C4
{
	// Effect messages
	enum EffectMessageType
	{
		kEffectMessageBeam,
		kEffectMessageBolt,
		kEffectMessageTargetted,
		kEffectMessagePositional,
		kEffectMessageNone
	};

	// Targetted Effects (must be applied to a BTEntity)
	enum TargettedEffects
	{
		kTargettedEffectMessageHeal = kEffectMessageNone + 1,
		kTargettedEffectMessageFlash,
		kTargettedEffectMessageSparks,
		kTargettedEffectMessageStun,
		kTargettedEffectMessageBladeCounter,
		kTargettedEffectMessageTurtleBarrier,
		kTargettedEffectMessagePoison,
		kTargettedEffectMessageVanish,
		kTargettedEffectMessageNone
	};

	// Position Effects (applied at a position)
	enum PositionalEffects
	{
		kPositionalEffectMessageNone = kTargettedEffectMessageNone + 1,
		kPositionalEffectMessageShockwave,
		kPositionalEffectMessageDepthCharge,
		kPositionalEffectMessageSmoke
	};

	// Controller types
	enum
	{
		kControllerBeamEffect			= 'beam'
	};

	/**
	 * The CreateEffectMessage is the base class for all effects.
	 * @author - Chris Williams
	 */
	class CreateEffectMessage : public Message
	{
		friend class Game;

		private:

			EffectMessageType		m_effectMessageType;
			long					m_lControllerIndex;

			CreateEffectMessage();

		public:

			CreateEffectMessage(EffectMessageType type, long lControllerIndex = -1);
			~CreateEffectMessage();

			EffectMessageType GetEffectMessageType(void) const
			{
				return (m_effectMessageType);
			}

			long GetControllerIndex(void) const
			{
				return (m_lControllerIndex);
			}

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);

			bool HandleMessage(Player *sender) const;

			static CreateEffectMessage *ConstructMessage(EffectMessageType type);
	};


	class CreateBeamEffectMessage : public CreateEffectMessage
	{
		friend class CreateEffectMessage;
		friend class Game;

		private:

			Point3D					m_p3dSourcePosition;

			float					m_fAltitude;
			float					m_fAzimuth;
			float					m_fDistance;

			char					m_cColor;

			CreateBeamEffectMessage( EffectMessageType type );

		public:

			CreateBeamEffectMessage(Point3D& p3dSourcePosition, float fAltitude, float fAzimuth, float fDistance, char color = 0);
			~CreateBeamEffectMessage();

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};

	class CreateBoltEffectMessage : public CreateEffectMessage
	{
		friend class CreateEffectMessage;
		friend class Game;

		private:

			Point3D				m_p3dSourcePosition;
			long				m_lTargetController;

			CreateBoltEffectMessage( EffectMessageType type );

		public:

			CreateBoltEffectMessage( Point3D& p3dSourcePosition, long lTargetController );
			~CreateBoltEffectMessage();

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};

	class CreatePositionalEffectMessage : public CreateEffectMessage
	{
		friend class CreateEffectMessage;
		friend class Game;

		private:

			PositionalEffects		m_effectPositionalType;

			Point3D					m_p3dSourcePosition;

			CreatePositionalEffectMessage( EffectMessageType type );

		public:

			CreatePositionalEffectMessage( PositionalEffects type, long lControllerIndex, Point3D& p3dSourcePosition );
			~CreatePositionalEffectMessage();

			Point3D GetSourcePosition( void ) const
			{
				return (m_p3dSourcePosition);
			}

			PositionalEffects GetPositionalType( void ) const
			{
				return (m_effectPositionalType);
			}

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};

	class CreateTargettedEffectMessage : public CreateEffectMessage
	{
		friend class CreateEffectMessage;
		friend class Game;

		private:

			TargettedEffects	m_effectTargettedType;

			long				m_lTargetController;

			CreateTargettedEffectMessage( EffectMessageType type );

		public:

			CreateTargettedEffectMessage( TargettedEffects type, long lControllerIndex, long lTargetController );
			~CreateTargettedEffectMessage();

			long GetTargetController( void ) const
			{
				return (m_lTargetController);
			}

			TargettedEffects GetTargettedType( void ) const
			{
				return (m_effectTargettedType);
			}

			void Compress(Compressor& data) const;
			bool Decompress(Decompressor& data);
	};

	// Beam Effect Controller - CW
	class BeamEffectController : public Controller
	{
		private:

			unsigned long			m_ulDuration;

			unsigned long			m_ulStartTime;

		public:

			BeamEffectController( unsigned long ulDuration );

			void Preprocess( void );

			void Move( void );
	};


	// Used by effects to remain at the position of its target - CW
	class AttachToEntityController : public Controller
	{
		private:

			Link<BTEntityController>		m_pEntity;
			Point3D							m_p3dOffset;

			bool							m_bMantainRotation;

		public:

			AttachToEntityController( BTEntityController *pEntity, bool bMantainRotation = false );
			~AttachToEntityController();

			void SetOffset( const Point3D& p3dOffset );

			void Preprocess( void );
			void Move( void );
	};

	class BladeCounterEffectController : public AttachToEntityController
	{
		private:

			float		m_fRotation;

		public:

			BladeCounterEffectController( BTEntityController *pEntity );

			void Preprocess( void );

			void Move( void );
	};
}

#endif
