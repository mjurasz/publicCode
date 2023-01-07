using System;
using System.Resources;
using System.Reflection;
using System.Globalization;
using System.Threading;

namespace net.jurasz.tools
{
	// /////////////////////////////////////////////////////////////////////////////////
	//								C l a s s											
	/// <summary>
	/// Klasa opakowujaca ResourceManager'a
	/// </summary>
	public class CResourceManagerEx : IResourceManagerEx
	{
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
		#region Definitions

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
		#region Members

		private static CultureInfo				sm_oCurrentUICulture	= CultureInfo.CurrentUICulture;
		private static readonly CDevelopTrace	def_oDTR				= new CDevelopTrace( "ResourceError", true, 0, 10 );

		private readonly ResourceManager	m_oResourceManager;
		private readonly bool				m_bUseCurrentThreadUICulture;

		private ENoMessageAction	m_eNoMsgAction = ENoMessageAction.eReturnNull;
		private bool				m_bCascadeResource;

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
		#region Constructors

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_oObj"></param>
		public CResourceManagerEx(
			CResourceManagerEx i_oObj
			) : this(
				i_oObj,
				false
				)
		{
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_oObj"></param>
		/// <param name="i_bUseCurrentThreadUICulture"></param>
		public CResourceManagerEx(
			CResourceManagerEx i_oObj,
			bool i_bUseCurrentThreadUICulture
			)
		{
			CPfxAssert.IsNotNull( i_oObj );
			m_oResourceManager = i_oObj.m_oResourceManager;
			m_eNoMsgAction = i_oObj.m_eNoMsgAction;
			m_bUseCurrentThreadUICulture = i_bUseCurrentThreadUICulture;
		}


		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_oResourceManager"></param>
		/// <param name="i_eNoMessageAction"></param>
		public CResourceManagerEx(
			ResourceManager i_oResourceManager,
			ENoMessageAction i_eNoMessageAction
			)
		{
			CPfxAssert.IsNotNull( i_oResourceManager );
			m_oResourceManager = i_oResourceManager;
			m_eNoMsgAction = i_eNoMessageAction;
		}


		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_oResourceManager"></param>
		public CResourceManagerEx(
			ResourceManager i_oResourceManager
			) : this(
				i_oResourceManager,
				ENoMessageAction.eReturnNull
				)
		{
		}


		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_oResourceSource"></param>
		/// <param name="i_eNoMessageAction"></param>
		public CResourceManagerEx(
			Type i_oResourceSource,
			ENoMessageAction i_eNoMessageAction
			) : this(
				i_oResourceSource,
				i_eNoMessageAction,
				false
				)
		{
			CPfxAssert.IsNotNull( i_oResourceSource );
			m_oResourceManager = new ResourceManager( i_oResourceSource );
			m_eNoMsgAction = i_eNoMessageAction;
		}


		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_oResourceSource"></param>
		/// <param name="i_eNoMessageAction"></param>
		/// <param name="i_bUseCurrentThreadUICulture"></param>
		public CResourceManagerEx(
			Type i_oResourceSource,
			ENoMessageAction i_eNoMessageAction,
			bool i_bUseCurrentThreadUICulture
			)
		{
			CPfxAssert.IsNotNull( i_oResourceSource );
			m_oResourceManager = new ResourceManager( i_oResourceSource );
			m_eNoMsgAction = i_eNoMessageAction;
			m_bUseCurrentThreadUICulture = i_bUseCurrentThreadUICulture;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sBaseName"></param>
		/// <param name="i_oAssembly"></param>
		/// <param name="i_eNoMessageAction"></param>
		public CResourceManagerEx(
			string i_sBaseName,
			Assembly i_oAssembly,
			ENoMessageAction i_eNoMessageAction
			) : this(
				i_sBaseName,
				i_oAssembly,
				i_eNoMessageAction,
				false
				)
		{
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sBaseName"></param>
		/// <param name="i_oAssembly"></param>
		/// <param name="i_eNoMessageAction"></param>
		/// <param name="i_bUseCurrentThreadUICulture"></param>
		public CResourceManagerEx(
			string i_sBaseName,
			Assembly i_oAssembly,
			ENoMessageAction i_eNoMessageAction,
			bool i_bUseCurrentThreadUICulture
			)
		{
			CPfxAssert.StringNotEmpty( i_sBaseName );
			CPfxAssert.IsNotNull( i_oAssembly );

			m_oResourceManager = new ResourceManager( i_sBaseName, i_oAssembly );
			m_eNoMsgAction = i_eNoMessageAction;
			m_bUseCurrentThreadUICulture = i_bUseCurrentThreadUICulture;
		}


		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sBaseName"></param>
		/// <param name="i_oAssembly"></param>
		/// <param name="i_oUsingResourceSet"></param>
		/// <param name="i_eNoMessageAction"></param>
		public CResourceManagerEx(
			string i_sBaseName,
			Assembly i_oAssembly,
			Type i_oUsingResourceSet,
			ENoMessageAction i_eNoMessageAction
			) : this(
				i_sBaseName,
				i_oAssembly,
				i_oUsingResourceSet,
				i_eNoMessageAction,
				false
				)
		{
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sBaseName"></param>
		/// <param name="i_oAssembly"></param>
		/// <param name="i_oUsingResourceSet"></param>
		/// <param name="i_eNoMessageAction"></param>
		/// <param name="i_bUseCurrentThreadUICulture"></param>
		public CResourceManagerEx(
			string i_sBaseName,
			Assembly i_oAssembly,
			Type i_oUsingResourceSet,
			ENoMessageAction i_eNoMessageAction,
			bool i_bUseCurrentThreadUICulture
			)
		{
			CPfxAssert.IsNotNull( i_oUsingResourceSet );
			CPfxAssert.StringNotEmpty( i_sBaseName );
			CPfxAssert.IsNotNull( i_oAssembly );

			m_oResourceManager = new ResourceManager( i_sBaseName, i_oAssembly, i_oUsingResourceSet );
			m_eNoMsgAction = i_eNoMessageAction;
			m_bUseCurrentThreadUICulture = i_bUseCurrentThreadUICulture;
		}


		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_oResourceSource"></param>
		/// <param name="i_oResourceSource"></param>
		public CResourceManagerEx(
			Type i_oResourceSource
			) : this(
				i_oResourceSource,
				ENoMessageAction.eReturnNull
				)
		{
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sBaseName"></param>
		/// <param name="i_oAssembly"></param>
		public CResourceManagerEx(
			string i_sBaseName,
			Assembly i_oAssembly
			) : this(
				i_sBaseName,
				i_oAssembly,
				ENoMessageAction.eReturnNull
				)
		{
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sBaseName"></param>
		/// <param name="i_oAssembly"></param>
		/// <param name="i_oUsingResourceSet"></param>
		public CResourceManagerEx(
			string i_sBaseName,
			Assembly i_oAssembly,
			Type i_oUsingResourceSet
			) : this(
				i_sBaseName,
				i_oAssembly,
				i_oUsingResourceSet,
				ENoMessageAction.eReturnNull
				)
		{
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
		#region PublicInterface

		/// <summary>
		/// 
		/// </summary>
		public bool p_bCascadeResource
		{
			get
			{
				return m_bCascadeResource;
			}

			set
			{
				m_bCascadeResource = value;
			}
		}


		/// <summary>
		/// 
		/// </summary>
		public ResourceManager p_oResourceManager
		{
			get
			{
				return m_oResourceManager;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		public ENoMessageAction p_eNoMsgAction
		{
			get
			{
				return m_eNoMsgAction;
			}

			set
			{
				m_eNoMsgAction = value;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		public bool p_bEnableDTR
		{
			set
			{
				def_oDTR.p_bLogOn = value;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		public static CultureInfo p_oCurrentUICulture
		{
			get
			{
				return sm_oCurrentUICulture;
			}

			set
			{
				sm_oCurrentUICulture = value;
			}
		}


		/// <summary>
		/// Wyciaga ³añcuch z zasobów
		/// </summary>
		/// <returns></returns>
		public string GetResourceString(
			string i_sMessageId
			)
		{
			try
			{
				CPfxAssert.StringNotEmpty( i_sMessageId );
				CPfxAssert.IsNotNull( m_oResourceManager );

				if (m_oResourceManager == null)
				{
					def_oDTR.Log( 5, "Brak obiektu ResourceManagera" );
					return EmptyMessage( i_sMessageId );
				}

				if (string.IsNullOrEmpty( i_sMessageId ))
				{
					def_oDTR.Log( 5, "Pusty identyfikator" );
					return EmptyMessage( i_sMessageId );
				}

				// ================================================================
				// Modified : 2007-12-14 12:04:00 by w.gladala
				// Comments : na potrzeby aplikacji Web - lokalizacja 
				//			  na podstawie ustawieñ jêzykowych przegladarki
				// ================================================================					
				CultureInfo oCultureInfo;
				if (m_bUseCurrentThreadUICulture)
				{
					oCultureInfo = Thread.CurrentThread.CurrentUICulture;
				}
				else
				{
					oCultureInfo = sm_oCurrentUICulture;
				}

				string sMessage = m_oResourceManager.GetString(
					i_sMessageId,
					oCultureInfo
					);


				if (sMessage == null)
				{
					if (!m_bCascadeResource)
					{
						def_oDTR.Log( 5, "ID({0}), Location({1}), Brak ³añcucha w zasobach", i_sMessageId, m_oResourceManager.BaseName );
					}
					return EmptyMessage( i_sMessageId );
				}
				return sMessage;
			}
			catch (Exception oException)
			{
				string sBaseName = "";
				if (m_oResourceManager != null)
				{
					sBaseName = m_oResourceManager.BaseName;
				}
				def_oDTR.Log( 5, "ID({0}), Location({1}), Wyjatek({2})", i_sMessageId, oException.Message, sBaseName );
				return EmptyMessage( i_sMessageId );
			}
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
		#region ProtectedInterface

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sMessageId"></param>
		/// <returns></returns>
		protected virtual string UserEmptyMessage(
			string i_sMessageId
			)
		{
			return "";
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sMessageId"></param>
		/// <returns></returns>
		private string EmptyMessage(
			string i_sMessageId
			)
		{
			try
			{
				switch (m_eNoMsgAction)
				{
					case ENoMessageAction.eReturnNull:
						return null;

					case ENoMessageAction.eReturnEmpty:
						return "";

					case ENoMessageAction.eReturnId:
						if (i_sMessageId != null)
						{
							return i_sMessageId;
						}
						return "";

					case ENoMessageAction.eReturnUserString:
						return UserEmptyMessage( i_sMessageId );

					default:
						CPfxAssert.Impossible( "Niemo¿liwy warunek switch(m_eNoMsgAction) z wartoœcia({0})", m_eNoMsgAction.ToString() );
						break;
				}
			}
			catch (Exception e)
			{
				CExceptionManager.LogException( e );
			}

			if (m_eNoMsgAction == ENoMessageAction.eReturnNull)
			{
				return null;
			}
			return "";
		}

		#endregion
	}
}