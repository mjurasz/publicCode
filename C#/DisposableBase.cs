using System;

namespace net.jurasz.tools
{
	// /////////////////////////////////////////////////////////////////////////////////
	//								C l a s s											
	/// <summary>
	/// Klasa bazowa klas implementujacych IDisposable
	/// </summary>
	/// <remarks>
	/// Klase mozna stosowac tylko jesli klasa potomna dziedziczy po object 
	/// (w przeciwnym przypadku IDisposable nalezy zaimplementowac
	/// bezposrednio w klasie potomnej - zgodnie ze schematem)
	/// </remarks>
	public abstract class CDisposableBase : IDisposable
	{
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
		#region Definitions
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
		#region Members
		
		/// <summary>
		/// Zmienna umozliwiajaca wylaczenie asercji w Finalizatorze
		/// </summary>
		private bool	m_bEnableAssertionInFinalizer = true;

		/// <summary>
		/// Zmienna informujaca, ze obiekt zostal juz zwolniony
		/// </summary>
		private bool	m_bDisposed = false;
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
		#region Constructors

		/// <summary>
		/// Finalizator
		/// </summary>
        ~CDisposableBase()
		{
			try
			{
				DisposePrv(false);
                if(m_bEnableAssertionInFinalizer)
                    CPfxAssert.Finalizer(this);
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
		#region PublicInterface

		/// <summary>
		/// Metoda zwalniajaca zasoby
		/// </summary>
		public void Dispose()
		{
			try
			{
				DisposePrv(true);
				GC.SuppressFinalize(this);
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}
		}

		/// <summary>
		/// 
		/// </summary>
		public bool p_bEnableAssertionInFinalizer
		{
			get
			{
				return m_bEnableAssertionInFinalizer;
			}
			set
			{
				m_bEnableAssertionInFinalizer = value;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		public bool WasDisposed()
		{
			return m_bDisposed;
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
		#region PrivateInterface

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_bDisposing"></param>
		protected abstract void Dispose(
			bool i_bDisposing
		);

		/// <summary>
		/// 
		/// </summary>
		protected void AssertNotCleanedUp()
		{
			CPfxAssert.Assert(!m_bDisposed, "Obiekt zostal juz zwolniony");
			if (m_bDisposed)
			{
				string sClassName = GetType().Name;
				throw new ObjectDisposedException(sClassName);
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		protected bool IsCleanedUp()
		{
			return m_bDisposed;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_bDisposing"></param>
		private void DisposePrv(
			bool i_bDisposing
		)
		{
			lock(this)
			{
				if (m_bDisposed)
					return;

				try
				{
					Dispose(i_bDisposing);
				}
				catch(Exception e)
				{
					CExceptionManager.LogException(e);
				}

				m_bDisposed = true;
			}
		}

		#endregion
	}
}
