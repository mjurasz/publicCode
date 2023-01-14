// ChooseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "geotec.h"
#include "MainFrm.h"
#include "DLG_Choose.h"
#include "DLG_DateRange.h"
#include "DLG_Progress.h"
#include "DLG_MeasDevChoice.h"
#include "ADO_ListaJednostekOrgZSekcjami.h"
#include "ADO_EvObLocalizations.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDLG_Choose dialog

CDLG_Choose :: CDLG_Choose(CWnd* pParent /*=NULL*/) : CDialog(CDLG_Choose :: IDD, pParent)
{
	m_nSelectedOrgUnit		= 0;
	m_nLastSerieMeasDevice	= TEC1435;
	m_nDefaultMeasSource	= TEC1435;
	m_nLastADOError			= 0;
//	m_bCancel				= FALSE;
	m_pDBEvReader			= NULL;
	m_pRootStorage			= NULL;
	m_bMeasDevChosen		= FALSE;
	m_bDontAskAboutSource	= FALSE;
#ifdef LOGOWANIE_JEDNOSTKI
	m_strUnitUser			= "";
#endif

	//{{AFX_DATA_INIT(CDLG_Choose)
	m_bWgId					= FALSE;
	m_strKmp				= _T("");
	m_strKmk				= _T("");
	m_bFiltr1				= FALSE;
	m_bFiltr2				= FALSE;
	//}}AFX_DATA_INIT
	m_oLastBegDate			= COleDateTime :: GetCurrentTime();
	m_oLastEndDate			= COleDateTime :: GetCurrentTime();

	m_nNbOfRoots			= 0;
	m_nNmbOfMeas			= 0;
	m_oInitBeginDate.SetDate(100, 1, 1);
	m_oInitEndDate.SetDate(9999, 1, 1);
	m_poaMeasSeries			= NULL;
	m_poaSortedSectors		= NULL;

	m_oBeginMeasDate.SetDate(100, 1, 1);
	m_oEndMeasDate.SetDate(9999, 1, 1);
	m_oBeginConstrDate.SetDate(100, 1, 1);
	m_oEndConstrDate.SetDate(9999, 1, 1);
	m_oBeginRepairDate.SetDate(100, 1, 1);
	m_oEndRepairDate.SetDate(9999, 1, 1);
	m_bKillFocusProcess		= FALSE;

	for (int i = 0; i < NB_OF_SERIES; i++)
		m_nSerieMeasDev[i] = -1;
}

void CDLG_Choose::DoDataExchange(CDataExchange* pDX)
{
	CDialog :: DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDLG_Choose)
	DDX_Control(pDX, IDC_COMBO2, m_cbOrgUnitsList);
	DDX_Control(pDX, IDC_COMBO1, m_cbTracksList);
	DDX_Control(pDX, IDC_EDIT2, m_ceKmk);
	DDX_Control(pDX, IDC_EDIT1, m_ceKmp);
	DDX_Control(pDX, IDC_LIST2, m_lbBudowa);
	DDX_Control(pDX, IDC_BTNMINUS, m_btnRem);
	DDX_Control(pDX, IDC_BTNPLUS, m_btnAdd);
	DDX_Control(pDX, IDC_LIST1, m_lbRanges);
	DDX_Check(pDX, IDC_WGIDKMTRACK, m_bWgId);
	DDX_Text(pDX, IDC_EDIT1, m_strKmp);
	DDX_Text(pDX, IDC_EDIT2, m_strKmk);
	DDX_Check(pDX, IDC_BTNFILTR, m_bFiltr1);
	DDX_Check(pDX, IDC_BTNFILTR2, m_bFiltr2);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_TREE1, m_ctBaseStruct);
}

BEGIN_MESSAGE_MAP(CDLG_Choose, CDialog)
	//{{AFX_MSG_MAP(CDLG_Choose)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE1, OnItemexpandingTree1)
	ON_BN_CLICKED(IDC_BTNPLUS, OnBtnplus)
	ON_BN_CLICKED(IDC_BTNMINUS, OnBtnminus)
	ON_BN_CLICKED(IDC_BTNFILTR2, OnBtnfiltr2)
	ON_BN_CLICKED(IDC_BTNPLUS2, OnBtnplus2)
	ON_BN_CLICKED(IDC_BTNMINUS2, OnBtnminus2)
	ON_BN_CLICKED(IDC_BTNFILTR, OnBtnfiltr)
	ON_BN_CLICKED(IDC_WGIDKMTRACK, OnWgidkmtrack)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, OnSelchangedTree1)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeList1)
	ON_LBN_SELCHANGE(IDC_LIST2, OnSelchangeList2)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnKillfocusEdit1)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnKillfocusEdit2)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, OnDblclkTree1)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList1)
	ON_CBN_SELCHANGE(IDC_COMBO2, OnSelchangeCombo2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDLG_Choose message handlers

BOOL CDLG_Choose :: OnInitDialog() 
{
	CDialog :: OnInitDialog();
	CString strTemp;
	ASSERT(m_poaMeasSeries != NULL);
	ASSERT(m_poaSortedSectors != NULL);
	
	m_bMeasDevChosen = TRUE;
	m_nLastSerieMeasDevice = TEC1435;

	#ifdef VERSION_SUBWAY_PL
		if (m_bDontAskAboutSource && (m_nDefaultMeasSource == TEC1435 || m_nDefaultMeasSource == SUBWAY_COMBO || m_nDefaultMeasSource == DTG1435))
		{
			m_bMeasDevChosen = TRUE;
			m_nLastSerieMeasDevice = m_nDefaultMeasSource;
		}
		else m_bMeasDevChosen = FALSE;
	#endif

	#ifdef COMPILE_MC
		if (m_bDontAskAboutSource && (m_nDefaultMeasSource == TEC1435 || m_nDefaultMeasSource == PLMC_EM120 || m_nDefaultMeasSource == DTG1435))
		{
			m_bMeasDevChosen = TRUE;
			m_nLastSerieMeasDevice = m_nDefaultMeasSource;
		}
		else m_bMeasDevChosen = FALSE;
	#endif

	#ifdef COMPILE_DTG
		if (m_bDontAskAboutSource && (m_nDefaultMeasSource == TEC1435 || m_nDefaultMeasSource == PLMC_EM120 || m_nDefaultMeasSource == DTG1435))
		{
			m_bMeasDevChosen = TRUE;
			m_nLastSerieMeasDevice = m_nDefaultMeasSource;
		}
		else m_bMeasDevChosen = FALSE;
	#endif

	AfxFormatString2(strTemp, IDS_BUDOWADATE, "----", "----");
	m_lbBudowa.AddString(strTemp);
	AfxFormatString2(strTemp, IDS_REMONTDATE, "----", "----");
	m_lbBudowa.AddString(strTemp);
		
	m_ctBaseStruct.SetImageList(NULL, 0);

	m_cbTracksList.EnableWindow(FALSE);
	m_ceKmp.EnableWindow(FALSE);
	m_ceKmk.EnableWindow(FALSE);
	
	m_oImageList.Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR24, 7, 0);

	m_oImageList.SetBkColor(GetSysColor(COLOR_WINDOW));

	m_ceKmp.SetMaxLen(12);
	m_ceKmk.SetMaxLen(12);
	HINSTANCE hInst;
	hInst = AfxGetResourceHandle();
	HICON hIcon;

	/* ladujemy wszystkie ikonki - urzadzen etc */
	for (int i = 0; i < 7; i++)
	{
		hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ILINE + i));
		ASSERT(hIcon != NULL);
		#ifdef	_DEBUG
			DWORD err = GetLastError();
		#endif
		m_nImageIdx[i] = m_oImageList.Add(hIcon);	
	}

	m_ctBaseStruct.SetImageList(&m_oImageList, TVSIL_NORMAL);

	/* wypelniamy liste jednostkami organizacyjnymi */
	FillOrgUnits();

	/* dodajemy linie do drzewa wyboru linii, odcinkow, sektorow ... */
	if (!AddLines())
		return FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja dodaje odcinki do drzewa wyboru linii, odcinkow, sektorow ....
// int nLine - identyfikator lini w bazie dla ktorej wczytywane sa odcinki
BOOL CDLG_Choose :: AddSections(int nLine, HTREEITEM hItemParent)
{
	int nCount;
	BOOL rv;
	HTREEITEM hItem;	

	/* tworzymy nowy obiekt */
	m_pListaOdcinkow = new CADO_ListaOdcinkow(&((CGeoTecApp *)AfxGetApp())->m_ADODb);

	HCURSOR hCur = :: SetCursor(LoadCursor(NULL, IDC_WAIT));
	try
	{
		/* przypisujemy parametr dla kwerendy */
		rv = m_pListaOdcinkow->m_pParameter->SetValue(nLine);

		/* jesli nie mamy zaznaczonej flagi, to znaczy ze wykrorzystujemy
		   kwerende po raz pierwszy i wtedy musimy "dodac" parametr */
		if (!m_pListaOdcinkow->m_bDodano)
		{
			rv = m_pListaOdcinkow->m_pCommand->AddParameter(m_pListaOdcinkow->m_pParameter);
			m_pListaOdcinkow->m_bDodano = TRUE;
			/* wykonujemy kwerende */
			rv = m_pListaOdcinkow->m_pRecord->Execute(m_pListaOdcinkow->m_pCommand);
		}
		/* wykonujemy kwerende po raz n-ty - tylko odswiezamy wynik */
		/* w przypadku jakichs dziwnych bledow - nalezy Requery wywolac tak:
		   m_pListaOdcinkow->m_pRecord->SetFilter(_T(""));
		   m_pListaOdcinkow->m_pRecord->GetRecordset()->Requery(adCmdTable); */ 
		else m_pListaOdcinkow->m_pRecord->Requery();

		/* jesli nie mamy zadnych wynikow */
		if (m_pListaOdcinkow->m_pRecord->IsBOF() && m_pListaOdcinkow->m_pRecord->IsEOF())
		{
			/* zamykamy polaczenie z baza */
			if (m_pListaOdcinkow->m_pRecord->IsOpen())
				m_pListaOdcinkow->m_pRecord->Close();

			::SetCursor(hCur);
			return TRUE;
		}

		/* pobieramy ilosc rekordow zwroconych przez kwerende */
		nCount = m_pListaOdcinkow->m_pRecord->GetRecordCount();

		CString strTemp, strKm, strFinal;

		/* dla kazdego rekordu wynikowego kwerendy */
		for (int i = 0; i < nCount; i++)
		{
			/* pobieramy wartosci */
			m_pListaOdcinkow->m_pRecord->GetFieldValue(_T("BEGKM"), (double)m_pListaOdcinkow->m_KmP);
			m_pListaOdcinkow->m_pRecord->GetFieldValue(_T("ENDKM"), (double)m_pListaOdcinkow->m_KmK);
			/* formatujemy string */
			strKm.Format(KM_FORMAT_IN_CHOOSEDLG, (double)m_pListaOdcinkow->m_KmP, (double)m_pListaOdcinkow->m_KmK);
			
			/* pobieramy wartosc */
			m_pListaOdcinkow->m_pRecord->GetFieldValue(_T("NAME"), m_pListaOdcinkow->m_Nazwa);
			/* formatujemy string */
			AfxFormatString2(strTemp, IDS_TREESECTION, (LPCTSTR)m_pListaOdcinkow->m_Nazwa, (LPCTSTR)strKm);
			
			/* pobieramy wartosc */
			m_pListaOdcinkow->m_pRecord->GetFieldValue(_T("TRACKID"), m_pListaOdcinkow->m_Track);
			/* formatujemy wartosci */
			strFinal.Format("%s%s", (LPCTSTR)strTemp, m_pListaOdcinkow->m_Track);
			
			/* tworzymy strukture, aby zapamietac w niej dane */
			m_pInfoRecSection = NULL;
			m_pInfoRecSection = new _DBSECTIONINFO;

			if (m_pInfoRecSection == NULL)
			{
				m_pListaOdcinkow->m_pRecord->Close();
				AfxMessageBox(IDS_MEMERROR, MB_OK | MB_ICONSTOP);
				:: SetCursor(hCur);
				return FALSE;
			}

			/* pobieramy wartosc */
			m_pListaOdcinkow->m_pRecord->GetFieldValue(_T("IDSECTION"), m_pListaOdcinkow->m_NrOdcinka);

			/* zapamietujemy dane */
			m_pInfoRecSection->nSecID		= m_pListaOdcinkow->m_NrOdcinka;
			m_pInfoRecSection->nfBegKm		= m_pListaOdcinkow->m_KmP;
			m_pInfoRecSection->nfEndKm		= m_pListaOdcinkow->m_KmK;
			m_pInfoRecSection->strTrackID	= m_pListaOdcinkow->m_Track;

			/* dla celow diagnostycznych wyswietlamy w trybie DEBUG ekstra informacje o numerze sekcji */
#ifdef _DEBUG
			CString strDEBUG;
			strDEBUG.Format(" [ SID: %d ]", m_pInfoRecSection->nSecID);
			strFinal += strDEBUG;
#endif

			/* dorzucamy do drzewka */
			hItem = NULL;
			hItem = m_ctBaseStruct.InsertItem((LPCTSTR)strFinal, m_nImageIdx[1], m_nImageIdx[1], hItemParent);

			/* sprawdzamy, czy sie udalo */
			if (hItem == NULL)
			{
				if (m_pInfoRecSection != NULL)	
					delete m_pInfoRecSection; 

				m_pInfoRecSection = NULL;
				m_pListaOdcinkow->m_pRecord->Close();

				AfxMessageBox(IDS_MEMERROR, MB_OK | MB_ICONSTOP);
				:: SetCursor(hCur);
				return FALSE;
			}
			
			m_ctBaseStruct.InsertItem("", hItem);
			/* przypisujemy do dodanego elementu drzewa zapisane w strukturze informacje */
			rv = m_ctBaseStruct.SetItemData(hItem, (DWORD)m_pInfoRecSection);

			/* idziemy do kolejnego rekordu */
			m_pListaOdcinkow->m_pRecord->MoveNext();
		}
		/* skonczylismy pobieranie danych, wiec zamykamy rekordset */
		m_pListaOdcinkow->m_pRecord->Close();
	}
	catch(CException* pErr)
	{
		ASSERT(pErr != NULL);
		if (pErr->IsKindOf(RUNTIME_CLASS(CMemoryException)))
		{
		#ifdef _DEBUG
			pErr->ReportError();
		#endif
			pErr->Delete();
			AfxMessageBox(IDS_MEMERROR, MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		CADOException* pADOErr = (CADOException *)pErr;
		(pADOErr->m_nCause != 0) ? (m_nLastADOError = pADOErr->m_nCause) : (m_nLastADOError = -1);
		
		pErr->Delete();
		pADOErr->Delete();
        
		CErrorProcess :: OnError(RGDBACCESSERROR, m_nLastADOError);

		try
		{
			if (m_pListaOdcinkow->m_pRecord->IsOpen())		
				m_pListaOdcinkow->m_pRecord->Close();
		}
		catch (CException* pErr)
		{
			pErr->Delete();
			return FALSE;
		}

		return FALSE;
	}
	:: SetCursor(hCur);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja dodaje sektory do drzewa wyboru linii, odcinkow, sektorow ....
// int nSection - identyfikator odcinka w bazie dla ktorego wczytywane sa sektory
BOOL CDLG_Choose :: AddSectors(int nSection, HTREEITEM hItemParent)
{
	int nCount;
	BOOL rv;
	HTREEITEM hItem;	

	/* tworzymy nowy obiekt */
	m_pListaSektorow = new CADO_ListaSektorow(&((CGeoTecApp *)AfxGetApp())->m_ADODb);

	/* okreslamy wartosci parametrow */
	m_pListaSektorow->m_pParameter1->SetValue(nSection);
	m_pListaSektorow->m_pParameter2->SetValue(m_oBeginConstrDate);
	m_pListaSektorow->m_pParameter3->SetValue(m_oEndConstrDate);
	m_pListaSektorow->m_pParameter4->SetValue(m_oBeginRepairDate);
	m_pListaSektorow->m_pParameter5->SetValue(m_oEndRepairDate);

	HCURSOR hCur = :: SetCursor(LoadCursor(NULL, IDC_WAIT));

	try
	{
		/* jesli jest to "pierwsze" uruchomienie kwerendy */
		if (!m_pListaSektorow->m_bDodano)
		{
			/* dodajemy parametr do komendy */
			m_pListaSektorow->m_pCommand->AddParameter(m_pListaSektorow->m_pParameter1);
			m_pListaSektorow->m_pCommand->AddParameter(m_pListaSektorow->m_pParameter2);
			m_pListaSektorow->m_pCommand->AddParameter(m_pListaSektorow->m_pParameter3);
			m_pListaSektorow->m_pCommand->AddParameter(m_pListaSektorow->m_pParameter4);
			m_pListaSektorow->m_pCommand->AddParameter(m_pListaSektorow->m_pParameter5);
			/* zaznaczamy ze do kwerendy juz dodano parametry */
			m_pListaSektorow->m_bDodano = TRUE;

			/* wykonujemy komende */
			m_pListaSektorow->m_pRecord->Execute(m_pListaSektorow->m_pCommand);
		}
		else
		{
			/* zmienilismy wartosci jakiegos parametru - odswiezamy wynik kwerendy */
			/* w przypadku jakichs dziwnych bledow - nalezy Requery wywolac tak:
				m_pListaSektorow->m_pRecord->SetFilter(_T(""));
				m_pListaSektorow->m_pRecord->GetRecordset()->Requery(adCmdTable); */ 
			m_pListaSektorow->m_pRecord->Requery();
		}

		CString strData = m_oBeginConstrDate.Format(_T("%A, %B %d, %Y"));
		strData = m_oEndConstrDate.Format(_T("%A, %B %d, %Y"));
		strData = m_oBeginRepairDate.Format(_T("%A, %B %d, %Y"));
		strData = m_oEndRepairDate.Format(_T("%A, %B %d, %Y"));

		/* jesli nie ma zadnych danych - kwerenda nie zwrocila nic */
		if (m_pListaSektorow->m_pRecord->IsBOF() && m_pListaSektorow->m_pRecord->IsEOF())
		{
			if (m_pListaSektorow->m_pRecord->IsOpen())
				m_pListaSektorow->m_pRecord->Close();
			:: SetCursor(hCur);

			return TRUE;
		}

		/* sprawdzamy ile rekordow zwrocila kwerenda */
		nCount = m_pListaSektorow->m_pRecord->GetRecordCount();

		CString strTemp, strKmp, strKmk;

		/* ... aby wykorzystac ja do sterowania petla - dla kazdej zwroconej przez nia wartosci */
		for (int i = 0; i < nCount; i++)
		{
			/* pobieramy wartosci pol aktualnego rekordu */
			m_pListaSektorow->m_pRecord->GetFieldValue(_T("BegKm"), m_pListaSektorow->m_KilometrP);
			m_pListaSektorow->m_pRecord->GetFieldValue(_T("EndKm"), m_pListaSektorow->m_KilometrK);
			m_pListaSektorow->m_pRecord->GetFieldValue(_T("IdSector"), m_pListaSektorow->m_IdSektora);
			m_pListaSektorow->m_pRecord->GetFieldValue(_T("BegPt"), m_pListaSektorow->m_PunktP);
			m_pListaSektorow->m_pRecord->GetFieldValue(_T("EndPt"), m_pListaSektorow->m_PunktK);

			/* formatujemy stringi */
			strKmp.Format(SINGLE_KM_GRID_FORMAT, (double)m_pListaSektorow->m_KilometrP);
			strKmk.Format(SINGLE_KM_GRID_FORMAT, (double)m_pListaSektorow->m_KilometrK);
			
			AfxFormatString2(strTemp, IDS_TREESECTORS, (LPCTSTR)strKmp, (LPCTSTR)strKmk);
			
			m_pInfoRecSector = NULL;
			m_pInfoRecSector = new _DBSECTORINFO;

			if (m_pInfoRecSector == NULL)
			{
				m_pListaSektorow->m_pRecord->Close();
				AfxMessageBox(IDS_MEMERROR, MB_OK | MB_ICONSTOP);
				:: SetCursor(hCur);
				return FALSE;
			}

			/* zapisujemy te dane w strukturze */
			m_pInfoRecSector->nSectorId = m_pListaSektorow->m_IdSektora;
			m_pInfoRecSector->nfBegKm	= m_pListaSektorow->m_KilometrP;
			m_pInfoRecSector->nfEndKm	= m_pListaSektorow->m_KilometrK;
			m_pInfoRecSector->nBegPt	= m_pListaSektorow->m_PunktP;
			m_pInfoRecSector->nEndPt	= m_pListaSektorow->m_PunktK;
			
			#ifdef _DEBUG
				CString strTemp2;
				strTemp2.Format(" - [ S: %d ]", m_pListaSektorow->m_IdSektora);
				strTemp += strTemp2;
			#endif

			/* wstawiamy do drzewka */
			hItem = NULL;
			hItem = m_ctBaseStruct.InsertItem((LPCTSTR)strTemp, m_nImageIdx[2], m_nImageIdx[2], hItemParent);
			
			/* jesli sie nie udalo ... */
			if (hItem == NULL)
			{
				if (m_pInfoRecSector != NULL)
					delete m_pInfoRecSector; 

				m_pInfoRecSector = NULL;
				m_pListaSektorow->m_pRecord->Close();
				AfxMessageBox(IDS_MEMERROR, MB_OK | MB_ICONSTOP);
				:: SetCursor(hCur);

				return FALSE;
			}
			m_ctBaseStruct.InsertItem("", hItem);

			/* wstawiamy wartosc do elementu */
			rv = m_ctBaseStruct.SetItemData(hItem, (DWORD)m_pInfoRecSector);
			
			/* idziemy do kolejnego rekordu */
			m_pListaSektorow->m_pRecord->MoveNext();
		}
		/* przeszlismy po wszystkich rekordach - zamykamy rekordset */
		m_pListaSektorow->m_pRecord->Close();
	}
	catch(CException* pErr)
	{
		ASSERT(pErr != NULL);
		if (pErr->IsKindOf(RUNTIME_CLASS(CMemoryException)))
		{
		#ifdef _DEBUG
			pErr->ReportError();
		#endif
			pErr->Delete();
			AfxMessageBox(IDS_MEMERROR, MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		CADOException* pADOErr = (CADOException *)pErr;
		(pADOErr->m_nCause != 0) ? (m_nLastADOError = pADOErr->m_nCause) : (m_nLastADOError = -1);
		
		pErr->Delete();
		pADOErr->Delete();
        
		CErrorProcess :: OnError(RGDBACCESSERROR, m_nLastADOError);

		try
		{
			if (m_pListaSektorow->m_pRecord->IsOpen())		
				m_pListaSektorow->m_pRecord->Close();
		}
		catch (CException* pErr)
		{
			pErr->Delete();
			return FALSE;
		}

		return FALSE;
	}
	:: SetCursor(hCur);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja dodaje pomiary do drzewa wyboru linii, odcinkow, sektorow ....
// int nSector - identyfikator sektora w bazie dla ktorego wczytywane sa pomiary
BOOL CDLG_Choose :: AddMeasurements(int nSector, HTREEITEM hItemParent, int nMeasDev)
{
	int nCount;
	BOOL rv;
	HTREEITEM hItem;	
	
	/* tworzymy nowy obiekt */
	m_pListaPomiarow = new CADO_ListaPomiarow(&((CGeoTecApp *)AfxGetApp())->m_ADODb, nMeasDev);

	/* ustawiamy wartosci parametrow dla kwerendy - numer sektora i daty poczatkowa i koncowa */
	m_pListaPomiarow->m_pParameter1->SetValue(nSector);
	m_pListaPomiarow->m_pParameter2->SetValue(m_oBeginMeasDate);
	m_pListaPomiarow->m_pParameter3->SetValue(m_oEndMeasDate);

	HCURSOR hCur = :: SetCursor(LoadCursor(NULL, IDC_WAIT));
	try
	{
		if (!m_pListaPomiarow->m_bDodano)
		{
			/* dodajemy parametr do komendy */
			rv = m_pListaPomiarow->m_pCommand->AddParameter(m_pListaPomiarow->m_pParameter1);
			rv = m_pListaPomiarow->m_pCommand->AddParameter(m_pListaPomiarow->m_pParameter2);
			rv = m_pListaPomiarow->m_pCommand->AddParameter(m_pListaPomiarow->m_pParameter3);
			m_pListaPomiarow->m_bDodano = TRUE;

			m_pListaPomiarow->m_pRecord->Execute(m_pListaPomiarow->m_pCommand);
		}
		else
		{
			/* wykonujemy komende */
			/* w przypadku jakichs dziwnych bledow - nalezy Requery wywolac tak:
			   m_pListaPomiarow->m_pRecord->SetFilter(_T(""));
			   m_pListaPomiarow->m_pRecord->GetRecordset()->Requery(adCmdTable); */ 
			m_pListaPomiarow->m_pRecord->Requery();
		}

		/* jesli nie ma nic w bazie co spelnia warunki zapisane w kwerendzie */
		if (m_pListaPomiarow->m_pRecord->IsBOF() && m_pListaPomiarow->m_pRecord->IsEOF())
		{
			if (m_pListaPomiarow->m_pRecord->IsOpen())
				m_pListaPomiarow->m_pRecord->Close();

			:: SetCursor(hCur);
			return TRUE;
		}

		/* pobieramy ilosc rekordow zwroconych przez kwerende */
		nCount = m_pListaPomiarow->m_pRecord->GetRecordCount();

		CString strTemp, strData;
		
		/* dla kazdego rekordu */
		for (int i = 0; i < nCount; i++)
		{
			/* pobieramy wartosc pola i formatujemy string */
			m_pListaPomiarow->m_pRecord->GetFieldValue(_T("DATEM"), m_pListaPomiarow->m_DataP);
			strData = m_pListaPomiarow->m_DataP.Format("%d.%m.%Y");
			AfxFormatString1(strTemp, IDS_TREEMEAS, (LPCTSTR)strData);
			
			m_pInfoRecMeas = NULL;
			m_pInfoRecMeas = new _DBMEASINFO;

			/* jesli nie udalo sie utworzyc struktury */
			if (m_pInfoRecMeas == NULL)
			{
				m_pListaPomiarow->m_pRecord->Close();
				AfxMessageBox(IDS_MEMERROR, MB_OK | MB_ICONSTOP);
				:: SetCursor(hCur);
				return FALSE;
			}

			/* pobieramy wartosc kolejnego pola */
			m_pListaPomiarow->m_pRecord->GetFieldValue(_T("IDMEAS"), m_pListaPomiarow->m_IdPomiaru);

			/* zapisujemy dane pobrane z kwerendy */
			m_pInfoRecMeas->nMeasID = m_pListaPomiarow->m_IdPomiaru;
			m_pInfoRecMeas->nSectorId = nSector;
			m_pInfoRecMeas->oDate = m_pListaPomiarow->m_DataP;
			m_pInfoRecMeas->nMeasDev = nMeasDev;
			
			/* w zaleznosci od urzadzenia zrodlowego - dodajemy do drzewa z odpowiednimi ustawieniami */
			hItem = NULL;
//Must be updated for every new device
			if (nMeasDev == TEC1435)
				hItem = m_ctBaseStruct.InsertItem((LPCTSTR)strTemp, m_nImageIdx[3], m_nImageIdx[3], hItemParent);

		/* dla DTG w wersji bd */
		#ifdef COMPILE_DTG
			else if (nMeasDev == DTG1435)
				    hItem = m_ctBaseStruct.InsertItem((LPCTSTR)strTemp, m_nImageIdx[5], m_nImageIdx[5], hItemParent);
		#endif

		/* dla urzadzenia TED ze Sheffield */
		#ifdef VERSION_SHEFFIELD
			else if (nMeasDev == TED1435)
				    hItem = m_ctBaseStruct.InsertItem((LPCTSTR)strTemp, m_nImageIdx[6], m_nImageIdx[6], hItemParent);
		#endif
				 else hItem = m_ctBaseStruct.InsertItem((LPCTSTR)strTemp, m_nImageIdx[4], m_nImageIdx[4], hItemParent);
//Must be updated for every new device			
			/* jesli nie udalo sie dodac do drzewka */
			if (hItem == NULL)
			{
				if (m_pInfoRecMeas != NULL)
					delete m_pInfoRecMeas; 
				m_pInfoRecMeas = NULL;
				m_pListaPomiarow->m_pRecord->Close();
				AfxMessageBox(IDS_MEMERROR, MB_OK | MB_ICONSTOP);
				:: SetCursor(hCur);
				return FALSE;
			}
			/* przypisujemy odpowiednie dane do elementu drzewka */
			rv = m_ctBaseStruct.SetItemData(hItem, (DWORD)m_pInfoRecMeas);

			/* przechodzimy do kolejnego rekordu zwroconego przez kwerende */			
			m_pListaPomiarow->m_pRecord->MoveNext();
		}
		/* zamykamy rekordset - pobralismy juz wszystko co bylo */
		m_pListaPomiarow->m_pRecord->Close();
	}
	catch (CException* pErr)
	{
		ASSERT(pErr != NULL);
		if (pErr->IsKindOf(RUNTIME_CLASS(CMemoryException)))
		{
		#ifdef _DEBUG
			pErr->ReportError();
		#endif
			pErr->Delete();
			AfxMessageBox(IDS_MEMERROR, MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		CADOException* pADOErr = (CADOException *)pErr;
		(pADOErr->m_nCause != 0) ? (m_nLastADOError = pADOErr->m_nCause) : (m_nLastADOError = -1);
		
		pErr->Delete();
		pADOErr->Delete();
        
		CErrorProcess :: OnError(RGDBACCESSERROR, m_nLastADOError);

		try
		{
			if (m_pListaPomiarow->m_pRecord->IsOpen())		
				m_pListaPomiarow->m_pRecord->Close();
		}
		catch (CException* pErr)
		{
			pErr->Delete();
			return FALSE;
		}
	}
	:: SetCursor(hCur);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja dodaje linie do drzewa wyboru linii, odcinkow, sektorow ....
BOOL CDLG_Choose :: AddLines()
{
	int nCount;
	BOOL rv;
	HTREEITEM hItem;
	m_nNbOfRoots = 0;

	/* tworzymy nowy obiekt */
	m_pListaLinii = new CADO_ListaLinii(&((CGeoTecApp *)AfxGetApp())->m_ADODb);

	HCURSOR hCur = :: SetCursor(LoadCursor(NULL, IDC_WAIT));
	try
	{
		/* sprawdzamy, ktora jednostka org. jest wybrana ... jesli ktoras konkretnie, to parametryzujemy wybor */
		if (m_cbOrgUnitsList.GetItemData(m_nSelectedOrgUnit) > 0)
		{
			/* przypisujemy wartosc parametru */
            rv = m_pListaLinii->m_pParameter->SetValue((int)m_cbOrgUnitsList.GetItemData(m_nSelectedOrgUnit));

			/* sprawdzamy, czy wczesniej juz parametr zostal dodany - inaczej bedzie blad */
			if (!m_pListaLinii->m_bDodano)
			{
				/* parametr nie byl jeszcze dodany, wiec robimy to */
				rv = m_pListaLinii->m_pCommand->AddParameter(m_pListaLinii->m_pParameter);
				/* i zaznaczamy ten fakt */
				m_pListaLinii->m_bDodano = TRUE;

				/* wykonujemy komende */
				rv = m_pListaLinii->m_pRecord->Execute(m_pListaLinii->m_pCommand);
			}
			/* parametr byl juz dodany, co oznacza, ze skoro juz zaktualizowalismy jego wartosc, 
			   to mozemy wykonac kwerende parametryzowana ponownie */
			/* w przypadku jakichs dziwnych bledow - nalezy Requery wywolac tak:
				m_pListaLinii->m_pRecord->SetFilter(_T(""));
				m_pListaLinii->m_pRecord->GetRecordset()->Requery(adCmdTable); */ 
			else m_pListaLinii->m_pRecord->Requery();
		}
		/* jesli jest to ogolny wybor - bierzemy kwerende nieparametryzowana */
		else 
		{
			/* Po prostu otwieramy zwykla kwerende */
			rv = m_pListaLinii->m_pRecord->Open(m_pListaLinii->GetDefaultSQL(), CADORecordset :: openQuery);
		}
		
		/* jesli nie ma nic w bazie - zamykamy polaczenie i wyswietlamy komunikat */
		if (m_pListaLinii->m_pRecord->IsBOF() && m_pListaLinii->m_pRecord->IsEOF())
		{
			if (m_pListaLinii->m_pRecord->IsOpen())
				m_pListaLinii->m_pRecord->Close();

			AfxMessageBox(IDS_DBEMPTYERROR, MB_OK | MB_ICONSTOP);
			:: SetCursor(hCur);

			return FALSE;
		}

		/* sprawdzamy ile jest rekordow wynikowych */
		nCount = m_pListaLinii->m_pRecord->GetRecordCount();

#ifdef _DEBUG
		int nDebugCount = m_pListaLinii->m_pRecord->GetFieldCount();
		CADOFieldInfo fild;
		nDebugCount = m_pListaLinii->m_pRecord->GetFieldInfo(0, &fild);
		nDebugCount = m_pListaLinii->m_pRecord->GetFieldInfo(1, &fild);
		nDebugCount = m_pListaLinii->m_pRecord->GetFieldInfo(2, &fild);
		nDebugCount = m_pListaLinii->m_pRecord->GetFieldInfo(3, &fild);
		nDebugCount = m_pListaLinii->m_pRecord->GetRecordCount();
#endif

		CString strTemp, strKm, strTmp, strTmp1;
		
		strTmp.LoadString(IDS_KM1);
		strTmp1.LoadString(IDS_TO);			
		
		/* dla kazdego wiersza w rekordsecie */
		for (int i = 0; i < nCount; i++)
		{
			/* pobieramy zwrocone z kwerendy wartosci i inicjalizujemy elementy drzewa dla drog */
			m_pListaLinii->m_pRecord->GetFieldValue(_T("BEGKM"), m_pListaLinii->m_km_p);
			m_pListaLinii->m_pRecord->GetFieldValue(_T("ENDKM"), m_pListaLinii->m_km_k);
			m_pListaLinii->m_pRecord->GetFieldValue(_T("NAME"), m_pListaLinii->m_nazwa);
			m_pListaLinii->m_pRecord->GetFieldValue(_T("ID_ROUTE"), m_pListaLinii->m_id_linii);			

#ifdef VERSION_TRAM_KRAKOW
			strTemp = m_pListaLinii->m_nazwa;
#else
	#ifdef VERSION_SHEFFIELD
			strKm.Format(" %.6f %s %s %.6f %s", (double)m_pListaLinii->m_km_p, (LPCTSTR)strTmp, 
						(LPCTSTR)strTmp1, (double)m_pListaLinii->m_km_k, (LPCTSTR)strTmp);
			AfxFormatString2(strTemp, IDS_TREELINE, (LPCTSTR)m_pListaLinii->m_nazwa, (LPCTSTR)strKm);
	#else
		#ifdef VERSION_UKTRAMS
			#ifdef TEC2WITHGROOVES
				strKm.Format(" %.6f %s %s %.6f %s", (double)m_pListaLinii->m_km_p, (LPCTSTR)strTmp, 
						(LPCTSTR)strTmp1, (double)m_pListaLinii->m_km_k, (LPCTSTR)strTmp);
			AfxFormatString2(strTemp, IDS_TREELINE, (LPCTSTR)m_pListaLinii->m_nazwa, (LPCTSTR)strKm);
			#endif
		#else
			strKm.Format(" %.3f %s %s %.3f %s", (double)m_pListaLinii->m_km_p, (LPCTSTR)strTmp, 
						(LPCTSTR)strTmp1, (double)m_pListaLinii->m_km_k, (LPCTSTR)strTmp);
			AfxFormatString2(strTemp, IDS_TREELINE, (LPCTSTR)m_pListaLinii->m_nazwa, (LPCTSTR)strKm);
		#endif
	#endif
#endif
			hItem = NULL;
			hItem = m_ctBaseStruct.InsertItem((LPCTSTR)strTemp, m_nImageIdx[0], m_nImageIdx[0]);
			
			if (hItem == NULL)
			{
				// m_pListaLinii->m_pRecord->Close();
				AfxMessageBox(IDS_MEMERROR, MB_OK | MB_ICONSTOP);
				:: SetCursor(hCur);
				return FALSE;
			}

			m_nNbOfRoots++;
			
			m_ctBaseStruct.InsertItem("", hItem);
			rv = m_ctBaseStruct.SetItemData(hItem, (DWORD)m_pListaLinii->m_id_linii);

			/* przechodzimy do kolejnego wiersza */
			m_pListaLinii->m_pRecord->MoveNext();
		}
		m_pListaLinii->m_pRecord->Close();
	}
	catch (CException* pErr)
	{
		ASSERT(pErr != NULL);
		if (pErr->IsKindOf(RUNTIME_CLASS(CMemoryException)))
		{
		#ifdef _DEBUG
			pErr->ReportError();
		#endif
			pErr->Delete();
			AfxMessageBox(IDS_MEMERROR, MB_OK | MB_ICONSTOP);
			return FALSE;
		}

		CADOException* pADOErr = (CADOException *)pErr;
		(pADOErr->m_nCause != 0) ? (m_nLastADOError = pADOErr->m_nCause) : (m_nLastADOError = -1);

		pErr->Delete();
		pADOErr->Delete();

		CErrorProcess :: OnError(RGDBACCESSERROR, m_nLastADOError);

		try
		{
			if (m_pListaLinii->m_pRecord->IsOpen())		
				m_pListaLinii->m_pRecord->Close();
		}
		catch (CException* pErr)
		{
			pErr->Delete();
			return FALSE;
		}

		return FALSE;
	}
	:: SetCursor(hCur);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja wywolywana jest po wcisnieciu ok, dalsze dzialanie przebiega nastepujaco:
// A) Jesli wybrano w oknie tryb wybierania torow poprzez wybor odcinka z bazy i wprowadzenie zakresu 
// 	  kilometraza ktory nalezy wczytac to :
//	  - funkcja sprawdza czy wpisany kilometraz poczatkowy i koncowy zakresu nie wykracza 
//	    poza kilometraz wybranego w liscie rozwijanej odcinka toru;
//	   - jesli tak to zglasza blad i wraca do okna wyboru;
//	   - jesli nie otwiera okno postepu wczytywania;
// B) Jesli droge rwybrano poprzez zaznaczanie lini, odcinkow ... w drzewie to wywolywane jest okno postepu
//	  wczytywania i przekazywany do drzewa jest obiekt CTreeCtrl w celu przeczytania wyboru uzytkownika
//    W oknie nastepuje analiza poprawnosci zaznaczonego fragmentu(ow) toru(ow) i jesli poprawnie 
//    zaznaczono fragment toru wczuytywane sa pomiary, jesli nie wyswietlany jest komunikat bledu i 
//	  nastepuje powrot do okna wyboru.
void CDLG_Choose :: OnOK() 
{
	HTREEITEM hItem;
	double	nfKmp = 0.0, nfKmk = 0.0;
	int		nBegKm = 0, nEndKm = 0;
	int		nImage = -1;
	BOOL	rv = FALSE;
	hItem = NULL;
	UpdateData(TRUE);
	CString strLineInfo = _T("");
	int		nSectionId = 0, nLine = 0;

	if (m_bWgId)
	{
		if (CheckKmp() == TRUE)
		{
			if (CheckKmk() == FALSE)
				return;
		}
		else return;
		hItem = m_ctBaseStruct.GetSelectedItem();
		if (hItem == NULL)
		{
			AfxMessageBox(IDS_SELECTLINE, MB_OK | MB_ICONINFORMATION);
			return;
		}
		rv = m_ctBaseStruct.GetItemImage(hItem, nImage, nImage);
		if (!rv)
		{
			AfxMessageBox(IDS_SELECTLINE, MB_OK | MB_ICONINFORMATION);
			return;
		}
		if (nImage != m_nImageIdx[0])  //is line selected ?
		{
			AfxMessageBox(IDS_SELECTLINE, MB_OK | MB_ICONINFORMATION);
			return;
		}

		nfKmp = atof((LPCTSTR)m_strKmp);
		nfKmk = atof((LPCTSTR)m_strKmk);
		
		nBegKm = CInformationProcess :: ScaleToInt(nfKmp, DB_MILSCALE);
		nEndKm = CInformationProcess :: ScaleToInt(nfKmk, DB_MILSCALE);		
	
		int nSel;
		nSel = m_cbTracksList.GetCurSel();

		if (nSel >= 0 && nSel < m_naTracksKms.GetSize() / 2)
			nSectionId = m_naTracksIds.GetAt(nSel);
		else return;
		
		m_nBegKm = nBegKm;
		m_nEndKm = nEndKm;

		strLineInfo = m_ctBaseStruct.GetItemText(hItem);
		nLine = m_ctBaseStruct.GetItemData(hItem);
	}
	
	m_nSelectedOrgUnit = m_cbOrgUnitsList.GetCurSel();

	CDLG_Progress progressDlg;
	m_poaSortedSectors->RemoveAll();

	if (FillTableOfMeasDates() == FALSE)
		return;

	SortTableOfMeasDates();

	// m_poaMeasSeries[0].oMeasTimeRange[0] - tu sa informacje o datach serii ?

	progressDlg.m_ptBaseStruct			= &m_ctBaseStruct;
	progressDlg.m_nNbOfRoots			= m_nNbOfRoots;
	progressDlg.m_poaSortedSectors		= m_poaSortedSectors;
	progressDlg.m_poaMeasSeries			= m_poaMeasSeries;
	progressDlg.m_nNbOfSeries			= m_nNmbOfMeas;

	progressDlg.m_oBeginConstrDate		= m_oBeginConstrDate;
	progressDlg.m_oEndConstrDate		= m_oEndConstrDate;
	progressDlg.m_oBeginRepairDate		= m_oBeginRepairDate;
	progressDlg.m_oEndRepairDate		= m_oEndRepairDate;
	progressDlg.m_psaLineAndSection		= m_psaLineAndSection;

	progressDlg.m_strLineInfo			= strLineInfo;
	progressDlg.m_bBySectionID			= m_bWgId;
	progressDlg.m_nBegKm				= nBegKm;
	progressDlg.m_nEndKm				= nEndKm;
	progressDlg.m_nSectionID			= nSectionId;
	progressDlg.m_nLineId				= nLine;
	progressDlg.m_pDBEvReader			= m_pDBEvReader;
	progressDlg.m_pRootStorage			= m_pRootStorage;
	progressDlg.m_nDefaultMeasSource	= m_nDefaultMeasSource;
	progressDlg.m_bDontAskAboutSource	= m_bDontAskAboutSource;
	progressDlg.m_bMeasDevChosen		= m_bMeasDevChosen;

#ifdef COMPILE_TEC2
	progressDlg.m_pnaDaneInsp			= m_pnaDaneInsp;
#endif

	try
	{
		if (progressDlg.DoModal() != RGOK)
		{
			m_poaSortedSectors->RemoveAll();
			return;
		}
	}
	catch(CException* pErr)
	{
		ASSERT(pErr != NULL);
		if (pErr->IsKindOf(RUNTIME_CLASS(CMemoryException)))
		{
			pErr->Delete();
			AfxMessageBox(IDS_MEMERROR, MB_OK | MB_ICONSTOP);
			return ;
		}
		CErrorProcess :: OnError(RGDBACCESSERROR, m_nLastADOError);
		return ;
	}

	m_nNbOfSectors			= progressDlg.m_nNbOfSectors;
	m_nDefaultMeasSource	= progressDlg.m_nDefaultMeasSource;
	m_bDontAskAboutSource	= progressDlg.m_bDontAskAboutSource;
	m_bMeasDevChosen		= progressDlg.m_bMeasDevChosen;
	m_bInverted				= progressDlg.m_bInverted;
	
	CDialog :: OnOK();
	//CDialog :: OnCancel();
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja usuwa podelementy zadanego elementu w drzewie
//Parametry:
//HTREEITEM item - rodzic ktorego podelementy sa usuwane
//bool bIsEmpty - flaga okreslajaca czy dodawany jest element pusty po usunieciu podelementow 
void CDLG_Choose :: RGEraseChildren(HTREEITEM item, bool bIsEmpty)
{
	HTREEITEM hChild, hChild1;

	hChild = m_ctBaseStruct.GetChildItem(item);
	int nImage;
	BOOL rv;
#ifdef _DEBUG
	CString str1, str2, str;
	str = m_ctBaseStruct.GetItemText(item);
#endif
	while (hChild)
	{
#ifdef _DEBUG
		str1 = m_ctBaseStruct.GetItemText(hChild);
#endif
		hChild1 = m_ctBaseStruct.GetChildItem(hChild);
#ifdef _DEBUG
		str2 = m_ctBaseStruct.GetItemText(hChild1);
#endif
        RGEraseChildren(hChild, bIsEmpty);

		rv = m_ctBaseStruct.GetItemImage(hChild, nImage, nImage);
		if (!rv)
		{
			m_ctBaseStruct.DeleteItem(hChild);
			hChild = m_ctBaseStruct.GetChildItem(item);
		}
		if (nImage == m_nImageIdx[1])		//if section
		{
			m_pInfoRecSection = (LPDBSECTIONINFO)m_ctBaseStruct.GetItemData(hChild);
			if (m_pInfoRecSection != NULL)
				delete m_pInfoRecSection;
			m_pInfoRecSection = NULL;
		}
		if (nImage == m_nImageIdx[2])		//if sector
		{
			m_pInfoRecSector = (LPDBSECTORINFO)m_ctBaseStruct.GetItemData(hChild);
			if(m_pInfoRecSector != NULL)
				delete m_pInfoRecSector;
			m_pInfoRecSector = NULL;
		}
		if (nImage == m_nImageIdx[3])		//if measurement
		{
			m_pInfoRecMeas = (LPDBMEASINFO)m_ctBaseStruct.GetItemData(hChild);
			if (m_pInfoRecMeas != NULL)
				delete m_pInfoRecMeas;
			m_pInfoRecMeas = NULL;
		}
		if (nImage == m_nImageIdx[4])		//if other device measurement
		{
			m_pInfoRecMeas = (LPDBMEASINFO)m_ctBaseStruct.GetItemData(hChild);
			if (m_pInfoRecMeas != NULL)
				delete m_pInfoRecMeas;
			m_pInfoRecMeas = NULL;
		}

		m_ctBaseStruct.DeleteItem(hChild);
		hChild = m_ctBaseStruct.GetChildItem(item);
	}

	if (bIsEmpty)
		m_ctBaseStruct.InsertItem("", item); 
}

void CDLG_Choose :: PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class

	CDialog :: PostNcDestroy();
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja dodaje odcinki, sektory lub pomiary do rozwijanego elementu w zaleznosci
// od poziomu elementu ktory jest rozwijany
void CDLG_Choose :: OnItemexpandingTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW *)pNMHDR;
	TV_ITEM item = pNMTreeView->itemNew;
	UINT nState;
	UINT newS = 0;
	int nImage;

	nState = TVIS_EXPANDEDONCE;
	newS = item.state;

	BOOL rv;
	if (m_bWgId)
	{
		*pResult = 1;
		return;
	}

	if ((newS & TVIS_EXPANDEDONCE) == 0)
	{
		rv = m_ctBaseStruct.GetItemImage(item.hItem, nImage, nImage);
		if (!rv)
		{
			*pResult = 1;
			return;
		}
		if (nImage == m_nImageIdx[0])	//Routes
		{
				RGEraseChildren(item.hItem, FALSE);
				rv = AddSections(m_ctBaseStruct.GetItemData(item.hItem), item.hItem);
				if (rv == FALSE)
				{
					m_ctBaseStruct.InsertItem("", item.hItem);
					*pResult = 1;	
					return ;
				}
		}
		if (nImage == m_nImageIdx[1])	//Sections
		{
				this->RGEraseChildren(item.hItem, FALSE);
				m_pInfoRecSection = (LPDBSECTIONINFO)m_ctBaseStruct.GetItemData(item.hItem);

				rv = AddSectors(m_pInfoRecSection->nSecID, item.hItem);
				if (rv == FALSE)
				{
					m_ctBaseStruct.InsertItem("", item.hItem);
					*pResult = 1;	
					return ;
				}
		}
		if (nImage == m_nImageIdx[2])	//Sectors
		{
				this->RGEraseChildren(item.hItem, FALSE);
				m_pInfoRecSector = (LPDBSECTORINFO)m_ctBaseStruct.GetItemData(item.hItem);

				rv = AddMeasurements(m_pInfoRecSector->nSectorId, item.hItem);
				if (rv == FALSE)
				{
					m_ctBaseStruct.InsertItem("", item.hItem);
					*pResult = 1;	
					return ;
				}

				#ifdef COMPILE_DTG
					rv = AddMeasurements(m_pInfoRecSector->nSectorId, item.hItem, DTG1435);
					if (rv == FALSE)
					{
						m_ctBaseStruct.InsertItem("", item.hItem);
						*pResult = 1;
						return;
					}
				#endif

				#ifdef COMPILE_MC	//defined
					rv = AddMeasurements(m_pInfoRecSector->nSectorId, item.hItem, PLMC_EM120);
					if (rv == FALSE)
					{
						m_ctBaseStruct.InsertItem("", item.hItem);
						*pResult = 1;
						return;
					}
				#endif

				#ifdef VERSION_SUBWAY_PL	//defined
					rv = AddMeasurements(m_pInfoRecSector->nSectorId, item.hItem, SUBWAY_COMBO);
					if (rv == FALSE)
					{
						m_ctBaseStruct.InsertItem("", item.hItem);
						*pResult = 1;
						return;
					}
				#endif

				#ifdef VERSION_SHEFFIELD
					rv = AddMeasurements(m_pInfoRecSector->nSectorId, item.hItem, TED1435);
					if (rv == FALSE)
					{
						m_ctBaseStruct.InsertItem("", item.hItem);
						*pResult = 1;
						return;
					}
				#endif
		}
	}
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja wywolywana po wcisnieciu przycisku dodawania serii pomiarowej
// Otwierane jest okno w ktorym wprowadza sie zakres czasu, 
// dla ktorego wczytywana jest seria oraz urzadzenie pomiarowe dla serii
void CDLG_Choose :: OnBtnplus() 
{
	CDLG_DateRange dateDlg;
	int nCount;
	CString strFrom, strTo;
	CString strFinal;
	dateDlg.m_oDateFrom = m_oLastBegDate;
	dateDlg.m_oDateTo = m_oLastEndDate;
	dateDlg.m_nMeasDevice = m_nLastSerieMeasDevice;

	HTREEITEM hItem = m_ctBaseStruct.GetSelectedItem();

	m_pInfoRecMeas = (LPDBMEASINFO)m_ctBaseStruct.GetItemData(hItem);

	ASSERT(m_pInfoRecMeas != NULL);

	/* teraz sprawdzamy, czy to jest element pomiaru (czyli nie posiada potomka) */
	if (!m_ctBaseStruct.GetChildItem(hItem))
		dateDlg.m_nMeasDevice = m_pInfoRecMeas->nMeasDev;
	
	dateDlg.m_bSeriesDate = TRUE;

	if (dateDlg.DoModal() == IDOK)
	{
		strFrom						= dateDlg.m_oDateFrom.Format("%d.%m.%Y");
		strTo						= dateDlg.m_oDateTo.Format("%d.%m.%Y");
		m_oLastBegDate				= dateDlg.m_oDateFrom;
		m_oLastEndDate				= dateDlg.m_oDateTo;
		m_nLastSerieMeasDevice		= dateDlg.m_nMeasDevice;
		nCount						= m_lbRanges.GetCount();
		ASSERT(nCount >= 0 && nCount < NB_OF_SERIES);
		m_nSerieMeasDev[nCount] = m_nLastSerieMeasDevice;
		AfxFormatString2(strFinal, IDS_TREERANGE, strFrom, strTo);

		#ifdef	COMPILE_MC
			CString strTemp;
			strTemp = CInformationProcess :: GetPrefixOrSufixSymbolOfMeasDev(m_nLastSerieMeasDevice);
			strTemp += strFinal;
			strFinal = strTemp;
		#endif

		#ifdef	VERSION_SUBWAY_PL
			CString strTemp;
			strTemp = CInformationProcess :: GetPrefixOrSufixSymbolOfMeasDev(m_nLastSerieMeasDevice);
			strTemp += strFinal;
			strFinal = strTemp;
		#endif

		m_lbRanges.AddString(strFinal);
		nCount = m_lbRanges.GetCount();

		if (nCount == NB_OF_SERIES)
			m_btnAdd.EnableWindow(FALSE);

		m_btnRem.EnableWindow(TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja usuwa serie dodana do listy wczytywanych hserii
void CDLG_Choose :: OnBtnminus() 
{
	int nSel;
	int nCount;
	nSel = m_lbRanges.GetCurSel();
	nCount = m_lbRanges.GetCount();
	
	if (nSel >= 0 && nSel < nCount)
	{
		for (int i = nSel + 1; i < nCount; i++)
			m_nSerieMeasDev[i - 1] = m_nSerieMeasDev[i];
		m_lbRanges.DeleteString(nSel);
		nCount = m_lbRanges.GetCount();
		if (nCount == 0)
			this->m_btnRem.EnableWindow(FALSE);
		if (nCount < NB_OF_SERIES)
			this->m_btnAdd.EnableWindow(TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja ustawia flage filtrowania drzewa lini, odcinkow ... wg zadanej daty pomiaru
void CDLG_Choose :: OnBtnfiltr()
{
	UpdateData(TRUE);

	if (m_bWgId)
	{
		m_bFiltr1 = FALSE;
		UpdateData(FALSE);
		return;
	}

	int nSel;
	int nCount;
	CString strSearched;
	CString strBegin, strEnd;
	nSel = m_lbRanges.GetCurSel();
	nCount = m_lbRanges.GetCount();
	int yb, mb, db;
	int ye, me, de;
	yb = 0; mb = 0; db = 0;
	ye = 0; me = 0; de = 0;

	if (m_bFiltr1)
		this->OnSelchangeList1();
	else
	{
		m_oBeginMeasDate = m_oInitBeginDate;
		m_oEndMeasDate = m_oInitEndDate;
		this->RefreshTree();
	}
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja zwraca ze stringu zakresu czasu postaci "16.10.2005 do 16.01.2006"
//stringi dat w postaci 16.10.2005 i 16.01.2006.
//Parametry:
//CString strSearched - string z zakresem czasu
//CString &strBegin, CString &strEnd - zwracana data poczatkowa i koncowa;
//Funkcja zwraca FALSE jesli format wejsciowy jest niepoprawny. 
BOOL CDLG_Choose :: GetDateBE(CString strSearched, CString &strBegin, CString &strEnd)
{
	int idx;
	idx = strSearched.Find('.');
	if (idx < 0 || idx > strSearched.GetLength()) 
		return FALSE;
	strBegin.Format("%s", (LPCTSTR)strSearched.Mid(idx - 2, 10));
	idx = strSearched.Find('.', idx + 10);
	if (idx < 0 || idx > strSearched.GetLength()) 
		return FALSE;
	strEnd.Format("%s", (LPCTSTR)strSearched.Mid(idx - 2, 10));
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja wywolywana po wcisnieciu przycisku zmiany zakresu daty filtrowania drzewa
//Otwierane jest okno w ktorym wprowadza sie zakres czasu, 
void CDLG_Choose :: OnBtnplus2() //zmien
{		
	int nSel;
	BOOL rv;
	CDLG_DateRange dateDlg;
	CString strDataB, strDataE, strSearched;
	CString strFrom, strTo;
	CString strFinal;

	nSel = m_lbBudowa.GetCurSel();
	int yb, mb, db;
	int ye, me, de;
		
	if (nSel >= 0 && nSel < 2)
	{	
		m_lbBudowa.GetText(nSel, strSearched);
		rv=GetDateBE(strSearched, strDataB, strDataE);
		if (rv == TRUE)
		{
			CInformationProcess :: SplitDate(strDataB, yb, mb, db);
			CInformationProcess :: SplitDate(strDataE, ye, me, de);
			dateDlg.m_oDateFrom.SetDate(yb, mb, db);
			dateDlg.m_oDateTo.SetDate(ye, me, de);

			if (dateDlg.DoModal() == IDOK)
			{
				strFrom = dateDlg.m_oDateFrom.Format("%d.%m.%Y");
				strTo = dateDlg.m_oDateTo.Format("%d.%m.%Y");
				if (nSel == 0)
					AfxFormatString2(strFinal, IDS_BUDOWADATE, strFrom, strTo);
				else
					AfxFormatString2(strFinal, IDS_REMONTDATE, strFrom, strTo);
				m_lbBudowa.DeleteString(nSel);
				m_lbBudowa.InsertString(nSel, strFinal);
			}
		}
		else
		{
			if (dateDlg.DoModal() == IDOK)
			{
				strFrom = dateDlg.m_oDateFrom.Format("%d.%m.%Y");
				strTo = dateDlg.m_oDateTo.Format("%d.%m.%Y");
				if(nSel == 0)
					AfxFormatString2(strFinal, IDS_BUDOWADATE, strFrom, strTo);
				else
					AfxFormatString2(strFinal, IDS_REMONTDATE, strFrom, strTo);
				m_lbBudowa.DeleteString(nSel);
				m_lbBudowa.InsertString(nSel, strFinal);
			}
		}
	}	
	if (m_bFiltr2)
	{
		RefreshTree();
	}
}		

/////////////////////////////////////////////////////////////////////////////////
//Funkcja wywolywana po wcisnieciu przycisku resetowania daty filtrowania
void CDLG_Choose :: OnBtnminus2() //usun
{	
	int nSel;
	CString strFinal;

	nSel = m_lbBudowa.GetCurSel();

	if (nSel >= 0 && nSel < 2)
	{	
		if (nSel == 0)
			AfxFormatString2(strFinal, IDS_BUDOWADATE, "----", "----");
		else
			AfxFormatString2(strFinal, IDS_REMONTDATE, "----", "----");

		m_lbBudowa.DeleteString(nSel);
		m_lbBudowa.InsertString(nSel,strFinal);
	}

	if (m_bFiltr2)
	{
		RefreshTree();
	}
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja wlaczajaca flage filtrowania drzewa wg remontu lub knstrukcji
void CDLG_Choose :: OnBtnfiltr2() 
{
	UpdateData(TRUE);

	if (m_bWgId)
	{
		m_bFiltr2 = FALSE;
		UpdateData(FALSE);
		return;
	}
	
	int yb, mb, db;
	int ye, me, de;
	CString strSearched;
	CString strDataB;
	CString strDataE;
	BOOL rv;

	if (m_bFiltr2)
	{
		m_lbBudowa.GetText(0, strSearched);
		rv=GetDateBE(strSearched, strDataB, strDataE);
		if (rv == TRUE)
		{
			CInformationProcess :: SplitDate(strDataB, yb, mb, db);
			CInformationProcess :: SplitDate(strDataE, ye, me, de);

			m_oBeginConstrDate.SetDate(yb, mb, db);
			m_oEndConstrDate.SetDate(ye, me, de);
		}
		else
		{
			m_oBeginConstrDate = m_oInitBeginDate;
			m_oEndConstrDate = m_oInitEndDate;
		}
		
		m_lbBudowa.GetText(1, strSearched);
		rv = GetDateBE(strSearched, strDataB, strDataE);
		if (rv == TRUE)
		{
			CInformationProcess :: SplitDate(strDataB, yb, mb, db);
			CInformationProcess :: SplitDate(strDataE, ye, me, de);
			m_oBeginRepairDate.SetDate(yb, mb, db);
			m_oEndRepairDate.SetDate(ye, me, de);
		}
		else
		{
			m_oBeginRepairDate = m_oInitBeginDate;
			m_oEndRepairDate = m_oInitEndDate;
		}
	}
	else
	{
		m_oBeginConstrDate = m_oInitBeginDate;
		m_oEndConstrDate = m_oInitEndDate;
		m_oBeginRepairDate = m_oInitBeginDate;
		m_oEndRepairDate = m_oInitEndDate;
	}
	this->RefreshTree();
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja zwija drzewo do pierwszego poziomu (widoczne tylko linie)
void CDLG_Choose::CollapseAllTree()
{
	HTREEITEM hItem;
	hItem = NULL;

	hItem = m_ctBaseStruct.GetRootItem();
	while (hItem != NULL)
	{
		m_ctBaseStruct.Expand(hItem, TVE_COLLAPSE);
		hItem = m_ctBaseStruct.GetNextSiblingItem(hItem);
	}
	hItem = m_ctBaseStruct.GetRootItem();
	if (hItem != NULL)
	{
		m_ctBaseStruct.Select(hItem, TVGN_CARET);
	}
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja kasuje elementy na drzewie
void CDLG_Choose :: RGRemoveAllTree()
{
	HTREEITEM hChild;

	hChild = m_ctBaseStruct.GetRootItem();
	while (hChild)
	{
		CString str;
		str = m_ctBaseStruct.GetItemText(hChild);

		this->RGEraseChildren(hChild,FALSE);
		m_ctBaseStruct.DeleteItem(hChild);
		hChild = m_ctBaseStruct.GetRootItem();
	}
}

BOOL CDLG_Choose :: DestroyWindow() 
{
	RGRemoveAllTree();	
	m_naTracksIds.RemoveAll();
	m_naTracksKms.RemoveAll();

	return CDialog :: DestroyWindow();
}
/////////////////////////////////////////////////////////////////////////////////
//Funkcja wywolywana po zmianie zaznaczenia elementu w drzewie.
//Jesli wlaczony jest tryb wyboru wg odcinka to dla zaznaczonej linii
//wypelniana jest lista odcinkow.
void CDLG_Choose :: OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW *)pNMHDR;
	// TODO: Add your control notification handler code here
	TV_ITEM item = pNMTreeView->itemNew;

	UINT newS = 0;
	int nImage;
	int nLine;
	
	newS = item.state;
	
	BOOL rv;
	CString str;
	if (m_bWgId)
	{
		rv = m_ctBaseStruct.GetItemImage(item.hItem,nImage,nImage);
		if (!rv)
		{
			*pResult = 0;
			return;
		}
		if (nImage == m_nImageIdx[0])  // Route selected
		{
			EmptyIdKmData();
			newS = item.state;
			if ((newS & TVIS_EXPANDEDONCE) == 0)
			{
				str = m_ctBaseStruct.GetItemText(item.hItem);
				nLine = m_ctBaseStruct.GetItemData(item.hItem);
				this->RGEraseChildren(item.hItem, FALSE);
				this->AddSections(nLine, item.hItem);
				FillTracksIds(item.hItem);
				m_ctBaseStruct.SetItemState(item.hItem, TVIS_EXPANDEDONCE, TVIS_EXPANDEDONCE);
			}
			else
			{
				str = m_ctBaseStruct.GetItemText(item.hItem);
				FillTracksIds(item.hItem);
			}
		}
		else
		{
			EmptyIdKmData();
		}
	}
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja wypelnia tablice identyfikatorow torow i wypelnia liste torow dla zaznaczonej linii w drzewie
//HTREEITEM hItemParent - uchwyt do zaznacznego elementu drzewa
void CDLG_Choose :: FillTracksIds(HTREEITEM hItemParent) 
{
	HTREEITEM hItem;

	hItem = m_ctBaseStruct.GetChildItem(hItemParent);
	CString str;
	UINT nBegKm, nEndKm;
	while (hItem)
	{
		m_pInfoRecSection = (LPDBSECTIONINFO)m_ctBaseStruct.GetItemData(hItem);
		if (m_pInfoRecSection != NULL)
		{
			m_naTracksIds.Add(m_pInfoRecSection->nSecID);
			
			nBegKm = CInformationProcess :: ScaleToInt(m_pInfoRecSection->nfBegKm, DB_MILSCALE);
			nEndKm = CInformationProcess :: ScaleToInt(m_pInfoRecSection->nfEndKm, DB_MILSCALE);		
			
			m_naTracksKms.Add(nBegKm);
			m_naTracksKms.Add(nEndKm);
			str = m_ctBaseStruct.GetItemText(hItem);
			m_cbTracksList.AddString(str);
			hItem = m_ctBaseStruct.GetNextSiblingItem(hItem);
		}
		else
			hItem = NULL;
	}
	m_cbTracksList.SetCurSel(0);
	OnSelchangeCombo1();
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja wywolywana jest po zmianie wyboru na liscie rozwijanej odcinkow.
//Pola edycji wypelniane sa kilometrazem poczatkowym i koncowym odcinka toru wybranego na liscie
void CDLG_Choose :: OnSelchangeCombo1() 
{
	// TODO: Add your control notification handler code here
	int nSel;
	nSel = m_cbTracksList.GetCurSel();
	int nBegKm, nEndKm;

	if (nSel >= 0 && nSel * 2 + 1 < m_naTracksKms.GetSize())
	{
		nBegKm = (int)m_naTracksKms.GetAt(nSel * 2);
		nEndKm = (int)m_naTracksKms.GetAt(nSel * 2 + 1);
		m_strKmp = CInformationProcess :: ConvertMileageToStringWithDBPrec(nBegKm, -1);
		m_strKmk = CInformationProcess :: ConvertMileageToStringWithDBPrec(nEndKm, -1);
		m_nCurKmp = nBegKm;
		m_nCurKmk = nEndKm;
	}	
	else
	{
		this->m_strKmp.Format("---");
		this->m_strKmk.Format("---");
	}
	UpdateData(FALSE);
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja czysci tablice identyfikatorow odcinkow i kilometrow poczatkowych i koncowych odcinkow
void CDLG_Choose :: EmptyIdKmData()
{
	m_naTracksKms.RemoveAll();
	m_naTracksIds.RemoveAll();
	m_cbTracksList.ResetContent();
	this->m_strKmp.Format("");
	this->m_strKmk.Format("");
	UpdateData(FALSE);
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja wypelnia tablice serii pomiarowych na podstawie informacji w liscie serii
BOOL CDLG_Choose :: FillTableOfMeasDates() 
{
	int nSel;
	int nCount;
	int yb, mb, db;
	int ye, me, de;
	CString strSearched;
	CString strDataB;
	CString strDataE;
	BOOL bIsMeasCar = FALSE;
	BOOL rv;
	
	nSel = 0;
	nCount = m_lbRanges.GetCount();
	m_nNmbOfMeas = nCount;
	
	while (nSel < nCount)
	{
		m_bMeasDevChosen = TRUE;
		m_lbRanges.GetText(nSel, strSearched);
		rv = GetDateBE(strSearched, strDataB, strDataE);
		if (rv == TRUE)
		{
			CInformationProcess :: SplitDate(strDataB, yb, mb, db);
			CInformationProcess :: SplitDate(strDataE, ye, me, de);
			m_oBeginMeasDate.SetDate(yb, mb, db);
			m_oEndMeasDate.SetDate(ye, me, de);
			ASSERT(nSel >= 0 && nSel < NB_OF_SERIES);
			m_poaMeasSeries[nSel].oMeasTimeRange[0] = m_oBeginMeasDate;
			m_poaMeasSeries[nSel].oMeasTimeRange[1] = m_oEndMeasDate;
			m_poaMeasSeries[nSel].nMeasDevice = m_nSerieMeasDev[nSel];
#ifdef COMPILE_MC
			bIsMeasCar = CheckIsThereMeasCar();
			// bIsMeasCar = (m_nSerieMeasDev[nSel] == PLMC_EM120) ? TRUE : FALSE;
#endif
		}
		nSel++;
	}
	if (nCount == 0)
	{
		m_nNmbOfMeas = 1;
		m_poaMeasSeries[0].oMeasTimeRange[0] = m_oBeginMeasDate;
		m_poaMeasSeries[0].oMeasTimeRange[1] = m_oEndMeasDate;
		m_poaMeasSeries[0].nMeasDevice = m_nSerieMeasDev[0];
#ifdef COMPILE_MC
		bIsMeasCar = CheckIsThereMeasCar();
		// bIsMeasCar = (m_nSerieMeasDev[0] == PLMC_EM120) ? TRUE : FALSE;
#endif
	}
	if (bIsMeasCar == TRUE)
	{
		::g_nMileageStoringScale = PLMC_MILSCALE;
		return TRUE;
	}

	if (m_nNmbOfMeas == 1)
	{
		/* jesli urzadzeniem jest DTG */
		if (m_poaMeasSeries[0].nMeasDevice == DTG1435)
			::g_nMileageStoringScale = DTG_MILSCALE;
		else if (m_poaMeasSeries[0].nMeasDevice == PLMC_EM120)
				::g_nMileageStoringScale = PLMC_MILSCALE;
	#ifdef VERSION_SHEFFIELD
		else if (m_poaMeasSeries[0].nMeasDevice == TED1435)
				::g_nMileageStoringScale = TED_MILSCALE;
	#else
		#ifdef VERSION_UKTRAMS
			#ifdef TEC2WITHGROOVES
				else ::g_nMileageStoringScale = TED_MILSCALE;
			#endif
		#else 
			else ::g_nMileageStoringScale = TEC_MILSCALE;
		#endif
	#endif
	}
	else 
	{
		BOOL bIsDTG = CheckIsThereDTG();
		if (bIsDTG)
			g_nMileageStoringScale = TEC_MILSCALE;
		
		#ifdef VERSION_UKTRAMS
			#ifdef TEC2WITHGROOVES
				g_nMileageStoringScale = TED_MILSCALE;
			#endif
		#else
			g_nMileageStoringScale = TEC_MILSCALE;
		#endif
	}

	return TRUE;
}

//////////////////////////////////////////////////////////
// Funckja sprawdza, czy ktoras seria pomiarowa
// zostala wykonana EM120
BOOL CDLG_Choose :: CheckIsThereMeasCar() 
{
	for (int i = 0; i < m_nNmbOfMeas; i++)
		if (m_nSerieMeasDev[i] == PLMC_EM120) 
			return TRUE;

	return FALSE;
}

//////////////////////////////////////////////////////////
// Funkcja sprawdza, czy ktoras seria pomiarowa
// zostala wykonana DTG
BOOL CDLG_Choose :: CheckIsThereDTG()
{
	for (int i = 0; i < m_nNmbOfMeas; i++)
		if (m_nSerieMeasDev[i] == DTG1435) 
			return TRUE;

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja wywolywana po zmianie zaznaczonej serii na liscie, w celu odswierzenia zawartosci 
//drzewa przy wlaczonym filtrowaniu
void CDLG_Choose :: OnSelchangeList1() 
{
	// TODO: Add your control notification handler code here
	int nSel;
	int nCount;
	int yb, mb, db;
	int ye, me, de;
	CString strSearched;
	CString strDataB;
	CString strDataE;
	BOOL rv;
	
	if (!m_bFiltr1)
		return;
	nSel = m_lbRanges.GetCurSel();
	nCount = m_lbRanges.GetCount();
	
	if (nSel >= 0 && nSel < nCount)
	{
		m_lbRanges.GetText(nSel, strSearched);
		rv = GetDateBE(strSearched, strDataB, strDataE);
		if (rv == TRUE)
		{
			CInformationProcess :: SplitDate(strDataB, yb, mb, db);
			CInformationProcess :: SplitDate(strDataE, ye, me, de);
			m_oBeginMeasDate.SetDate(yb, mb, db);
			m_oEndMeasDate.SetDate(ye, me, de);
		}
	}
	else
	{
		m_oBeginMeasDate = m_oInitBeginDate;
		m_oEndMeasDate = m_oInitEndDate;
	}
	if (m_bFiltr1)
		RefreshTree();
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja usuwa wszystkie elementy z drzewa i ponownie wypelnia drzewo 
// (wykorzystwana przy wlaczonym filtrowaniu)
void CDLG_Choose :: RefreshTree()
{
	HCURSOR hCur;
	hCur = ::SetCursor(LoadCursor(NULL, IDC_WAIT));
	m_ctBaseStruct.ShowWindow(SW_HIDE);
	RGRemoveAllTree();
	AddLines();
	m_ctBaseStruct.ShowWindow(SW_SHOW);
	::SetCursor(hCur);
}

void CDLG_Choose :: OnSelchangeList2() 
{
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja wywolywana po utracie focus-a przez pole kilometrazu poczatkowego 
//(weryfikacja czy kilometraz w zakresie odcinka)
void CDLG_Choose :: OnKillfocusEdit1() 
{
	return;
	CWnd *pWnd;

	int nID;
	pWnd = NULL;
	pWnd = GetFocus();
	if (pWnd == NULL)
		return;
	nID = pWnd->GetDlgCtrlID();

	if (nID == IDCANCEL || nID == 0)
		return;

	if (!m_bKillFocusProcess)
	{
		CheckKmp();
	}
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja sprawdza czy kilometraz poczatkowy znajduje sie w zakresie wybranego odcinka
BOOL CDLG_Choose :: CheckKmp() 
{
	double nfKmp, nfKmk;
	int nBegKm, nEndKm;
	CString strMessage, strKmp, strKmk;

	m_bKillFocusProcess = TRUE;
	UpdateData(TRUE);
	
	nfKmp = atof((LPCTSTR)m_strKmp);
	nfKmk = atof((LPCTSTR)m_strKmk);
	
	nBegKm = CInformationProcess :: ScaleToInt(nfKmp, DB_MILSCALE);
	nEndKm = CInformationProcess :: ScaleToInt(nfKmk, DB_MILSCALE);
	
	if (m_nCurKmp < m_nCurKmk)
	{
		strKmp = CInformationProcess :: ConvertMileageToStringWithDBPrec(m_nCurKmp, -1);
		strKmk = CInformationProcess :: ConvertMileageToStringWithDBPrec(m_nCurKmk, -1);

		if (nBegKm >= m_nCurKmp && nBegKm <= m_nCurKmk)
		{
			if (nBegKm>nEndKm)
			{
				m_strKmp = CInformationProcess :: ConvertMileageToStringWithDBPrec(nEndKm, -1);
				m_strKmk = CInformationProcess :: ConvertMileageToStringWithDBPrec(nBegKm, -1);
				UpdateData(FALSE);
			}
			m_bKillFocusProcess = FALSE;
			m_bDataOK = TRUE;
		 	return TRUE;
		}
		else
			AfxFormatString2(strMessage, IDS_OVERRANGE, strKmp, strKmk);
	}
	if (m_nCurKmp > m_nCurKmk)
	{
		strKmk = CInformationProcess :: ConvertMileageToStringWithDBPrec(m_nCurKmp, -1);
		strKmp = CInformationProcess :: ConvertMileageToStringWithDBPrec(m_nCurKmk, -1);

		if (nBegKm >= m_nCurKmk && nBegKm <= m_nCurKmp)
		{
			if(nBegKm < nEndKm)
			{
				m_strKmp = CInformationProcess :: ConvertMileageToStringWithDBPrec(nEndKm, -1);
				m_strKmk = CInformationProcess :: ConvertMileageToStringWithDBPrec(nBegKm, -1);
				UpdateData(FALSE);
			}
			m_bKillFocusProcess = FALSE;
			m_bDataOK = TRUE;
		 	return TRUE;
		}
		else
			AfxFormatString2(strMessage, IDS_OVERRANGE, strKmp, strKmk);
	}
	AfxMessageBox(strMessage, MB_OK | MB_ICONINFORMATION);
	this->m_ceKmp.SetFocus();
	m_bKillFocusProcess = FALSE;
	m_bDataOK = FALSE;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja wywolywana po utracie focus-a przez pole kilometrazu koncowego
//(weryfikacja czy kilometraz w zakresie odcinka)
void CDLG_Choose :: OnKillfocusEdit2() 
{
	return;
	CWnd *pWnd;
	int nID;
	pWnd = NULL;
	pWnd = GetFocus();

	if (pWnd == NULL)
		return;
	
	nID = pWnd->GetDlgCtrlID();
	if (nID == IDCANCEL || nID == 0)
		return;

	if (!m_bKillFocusProcess)
	{
		CheckKmk();
	}
}
/////////////////////////////////////////////////////////////////////////////////
//Funkcja sprawdza czy kilometraz koncowy znajduje sie w zakresie wybranego odcinka
BOOL CDLG_Choose :: CheckKmk() 
{
	double nfKmp, nfKmk;
	int nBegKm, nEndKm;
	CString strMessage, strKmp, strKmk;

	m_bKillFocusProcess = TRUE;
	UpdateData(TRUE);
	nfKmp = atof((LPCTSTR)m_strKmp);
	nfKmk = atof((LPCTSTR)m_strKmk);
	
	nBegKm = CInformationProcess :: ScaleToInt(nfKmp, DB_MILSCALE);	
	nEndKm = CInformationProcess :: ScaleToInt(nfKmk, DB_MILSCALE);	
	
	if (m_nCurKmp < m_nCurKmk)
	{
		strKmp = CInformationProcess :: ConvertMileageToStringWithDBPrec(m_nCurKmp, -1);
		strKmk=CInformationProcess :: ConvertMileageToStringWithDBPrec(m_nCurKmk, -1);

		if (nEndKm >= m_nCurKmp && nEndKm <= m_nCurKmk)
		{
			if (nBegKm>nEndKm)
			{
				m_strKmp = CInformationProcess :: ConvertMileageToStringWithDBPrec(nEndKm, -1);
				m_strKmk = CInformationProcess :: ConvertMileageToStringWithDBPrec(nBegKm, -1);
				UpdateData(FALSE);
			}
			m_bKillFocusProcess = FALSE;
			m_bDataOK = TRUE;
		 	return TRUE;
		}
		else
			AfxFormatString2(strMessage, IDS_OVERRANGE, strKmp, strKmk);
	}
	if (m_nCurKmp > m_nCurKmk)
	{
		strKmk = CInformationProcess :: ConvertMileageToStringWithDBPrec(m_nCurKmp, -1);
		strKmp = CInformationProcess :: ConvertMileageToStringWithDBPrec(m_nCurKmk, -1);

		if (nEndKm >= m_nCurKmk && nEndKm <= m_nCurKmp)
		{
			if (nBegKm<nEndKm)
			{
				m_strKmp = CInformationProcess :: ConvertMileageToStringWithDBPrec(nEndKm, -1);
				m_strKmk = CInformationProcess :: ConvertMileageToStringWithDBPrec(nBegKm, -1);
				UpdateData(FALSE);
			}
			m_bKillFocusProcess = FALSE;
			m_bDataOK = TRUE;
		 	return TRUE;
		}
		else
			AfxFormatString2(strMessage, IDS_OVERRANGE, strKmp, strKmk);
	}
	AfxMessageBox(strMessage, MB_OK | MB_ICONINFORMATION);
	this->m_ceKmk.SetFocus();
	m_bKillFocusProcess = FALSE;
	m_bDataOK = FALSE;
	return FALSE;
}

void CDLG_Choose :: OnCancel() 
{
//	m_bCancel=TRUE;
	CDialog :: OnCancel();
}

void CDLG_Choose :: OnClose() 
{
	CDialog::OnClose();
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja sortuje tablice serii wg daty (od najnowszego do najstarszego)
int CDLG_Choose :: SortTableOfMeasDates()
{

	COleDateTime oDateB, oDateE;
	_SERIEINFORMATION oStoredSerie;

	for (int i = 0; i < m_nNmbOfMeas; i++)
	{
		for (int j = 0; j < m_nNmbOfMeas; j++)
		{
			oStoredSerie = m_poaMeasSeries[i];
			if (m_poaMeasSeries[j].oMeasTimeRange[1] < m_poaMeasSeries[i].oMeasTimeRange[1])
			{
				m_poaMeasSeries[i] = m_poaMeasSeries[j];
				m_poaMeasSeries[j] = oStoredSerie;
				oStoredSerie = m_poaMeasSeries[i];
			}
		}
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja wywoluje okno dodawania serii pomiarowej jesli kliknieto dwukrotnie na pomiarze dla sektora
// Domyslnie wpisywana jest data pmiary na ktorym kliknieto jako poczatek i koniec zakresu
void CDLG_Choose :: OnDblclkTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	BOOL rv;
	int nImage;
	HTREEITEM hItem;

	if (!m_bWgId)
	{
		hItem = m_ctBaseStruct.GetSelectedItem();
		if (hItem == NULL)
		{
			*pResult = 1;
			return;
		}
		rv = m_ctBaseStruct.GetItemImage(hItem, nImage, nImage);
		if (!rv)
		{
			*pResult = 1;
			return;
		}
		/* jesli jedno z obslugiwanych urzadzen */
		if (nImage == m_nImageIdx[3] || nImage == m_nImageIdx[4] || nImage == m_nImageIdx[5] || nImage == m_nImageIdx[6])	// Pomiary
		{
			m_pInfoRecMeas = (LPDBMEASINFO)m_ctBaseStruct.GetItemData(hItem);
			ASSERT(m_pInfoRecMeas != NULL);
			AskForMeasDateRange(m_pInfoRecMeas->oDate, m_pInfoRecMeas->nMeasDev);
		}
	}
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja wyswietla okno dodawania serii pomiarowej w celu pobrania zakresu dla serii
// Domyslnie wpisywana jest data zadana parametrem oSugestedDate i urzadzenie nMeasDev
void CDLG_Choose :: AskForMeasDateRange(COleDateTime oSugestedDate, int nMeasDev)
{
	CDLG_DateRange dateDlg;
	int nCount;
	CString strFrom, strTo;
	CString strFinal;
	dateDlg.m_oDateFrom = oSugestedDate;
	dateDlg.m_oDateTo = oSugestedDate;
	dateDlg.m_nMeasDevice = nMeasDev;
	nCount = m_lbRanges.GetCount();

	if (nCount >= NB_OF_SERIES)
		return;

	if (dateDlg.DoModal() == IDOK)
	{
		strFrom = dateDlg.m_oDateFrom.Format("%d.%m.%Y");
		strTo = dateDlg.m_oDateTo.Format("%d.%m.%Y");
		m_oLastBegDate = dateDlg.m_oDateFrom;
		m_oLastEndDate = dateDlg.m_oDateTo;
		m_nLastSerieMeasDevice = dateDlg.m_nMeasDevice;
		ASSERT(nCount >= 0 && nCount < NB_OF_SERIES);
		m_nSerieMeasDev[nCount] = m_nLastSerieMeasDevice;
		AfxFormatString2(strFinal, IDS_TREERANGE, strFrom, strTo);

		CString strTemp;

#ifdef	COMPILE_MC
		strTemp = CInformationProcess :: GetPrefixOrSufixSymbolOfMeasDev(m_nLastSerieMeasDevice);
		strTemp += strFinal;
		strFinal = strTemp;
#endif

#ifdef	VERSION_SUBWAY_PL
		strTemp = CInformationProcess :: GetPrefixOrSufixSymbolOfMeasDev(m_nLastSerieMeasDevice);
		strTemp += strFinal;
		strFinal = strTemp;
#endif

		m_lbRanges.AddString(strFinal);
		nCount = m_lbRanges.GetCount();

		if (nCount == NB_OF_SERIES)
			m_btnAdd.EnableWindow(FALSE);
		m_btnRem.EnableWindow(TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja wyswietla okno w ktorym mozna wybrac domyslne urzadzenie dla serii
void CDLG_Choose :: OnButton1() 
{	
	CDLG_MeasDevChoice oMeasDevChoiceDlg;
	
	oMeasDevChoiceDlg.m_bUseAsDefault = m_bDontAskAboutSource;//=FALSE;
	oMeasDevChoiceDlg.m_nMeasDevice = m_nDefaultMeasSource;
	if (oMeasDevChoiceDlg.DoModal() == IDOK)
	{
		m_nLastSerieMeasDevice = oMeasDevChoiceDlg.m_nMeasDevice;
		m_bDontAskAboutSource = oMeasDevChoiceDlg.m_bUseAsDefault;
		m_nDefaultMeasSource = m_nLastSerieMeasDevice;
		m_bMeasDevChosen = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja wywylywana po zaznaczeniu/odznaczeniu pola wyboru sposobu wybierania drogi 
//do wczytywania pomiarow
//Funkcja zwija drzewo do poziomu linii, i wlacza tryb wyboru wg odcinka 
//(uaktywnia liste torow i pola zakresu kilometrazu),
//lub deaktywuje jesli wylaczono tryb wyboru wg odcinkow
void CDLG_Choose :: OnWgidkmtrack() 
{
	UpdateData(TRUE);
	if (m_bWgId == TRUE)
	{
		m_bFiltr1 = FALSE;
		m_bFiltr2 = FALSE;
		UpdateData(FALSE);
		this->m_cbTracksList.EnableWindow(TRUE);
		this->m_ceKmp.EnableWindow(TRUE);
		this->m_ceKmk.EnableWindow(TRUE);
		HTREEITEM hSelectedRoute = NULL;
		hSelectedRoute = GetSelectedRoute();
		CollapseAllTree();
		if (hSelectedRoute != NULL)
			FillSectionCombo(hSelectedRoute);
		m_bDataOK = FALSE;
	}
	else
	{
		EmptyIdKmData();
		this->m_cbTracksList.EnableWindow(FALSE);
		this->m_ceKmp.EnableWindow(FALSE);
		this->m_ceKmk.EnableWindow(FALSE);
		m_bDataOK = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja zwraca uchwyt do zaznaczonej linii w drzewie
HTREEITEM CDLG_Choose :: GetSelectedRoute()
{
	HTREEITEM hItem = NULL;
	int nImage = -1;
	BOOL rv;

	hItem = m_ctBaseStruct.GetSelectedItem();
	if (hItem)
	{
		rv = m_ctBaseStruct.GetItemImage(hItem, nImage, nImage);
		ASSERT(rv);
		while (nImage != m_nImageIdx[0] && hItem != NULL)  //Route image
		{
			hItem = m_ctBaseStruct.GetParentItem(hItem);
			rv = m_ctBaseStruct.GetItemImage(hItem, nImage, nImage);
		}
		if (nImage != m_nImageIdx[0])
			hItem = NULL;
	}
	return hItem;
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja wypelnia liste rowijana odcinkow torow na podstawie linii na ktora wskazuje 
//uchwyt przekazany jako parametr (HTREEITEM hRouteItem)
void CDLG_Choose :: FillSectionCombo(HTREEITEM hRouteItem)
{
	int nNewState, nLineID;
	ASSERT(hRouteItem != NULL);

	EmptyIdKmData();
	nNewState = m_ctBaseStruct.GetItemState(hRouteItem, 0xFFFFFFFF);
	if ((nNewState & TVIS_EXPANDEDONCE) == 0)
	{
		nLineID = m_ctBaseStruct.GetItemData(hRouteItem);
		this->RGEraseChildren(hRouteItem, FALSE);
		this->AddSections(nLineID, hRouteItem);
		FillTracksIds(hRouteItem);
		m_ctBaseStruct.SetItemState(hRouteItem, TVIS_EXPANDEDONCE, TVIS_EXPANDEDONCE);
		m_ctBaseStruct.SelectItem(hRouteItem);
		return;
	}
	FillTracksIds(hRouteItem);
	m_ctBaseStruct.SelectItem(hRouteItem);
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja otwiera okno w celu edycji zakresu czasu dla serii po dwukrotnym kliknieciu na serii na liscie serii
void CDLG_Choose :: OnDblclkList1() 
{
	// TODO: Add your control notification handler code here
	AskForEditSelectedMeasDateRange();
}
/////////////////////////////////////////////////////////////////////////////////
//Funkcja otwiera okno w celu edycji zakresu czasu dla serii
void CDLG_Choose :: AskForEditSelectedMeasDateRange()
{
	CDLG_DateRange dateDlg;
	int nSelected;
	
	nSelected = GetSelectedDateRangInfo(dateDlg.m_oDateFrom, dateDlg.m_oDateTo, dateDlg.m_nMeasDevice);

	if (nSelected == LB_ERR)
		return;

	dateDlg.m_bSeriesDate = TRUE;
	if (dateDlg.DoModal() == IDOK)
	{
		m_oLastBegDate = dateDlg.m_oDateFrom;
		m_oLastEndDate = dateDlg.m_oDateTo;
		m_nLastSerieMeasDevice = dateDlg.m_nMeasDevice;
		SetDateRangeInfoList(nSelected, dateDlg.m_oDateFrom, dateDlg.m_oDateTo, m_nLastSerieMeasDevice);
	}
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja zwraca zakres czasu dla serii oraz identyfikator urzadzenia dla serii zaznaczonej na liscie
int CDLG_Choose :: GetSelectedDateRangInfo(COleDateTime &oDateFrom, COleDateTime &oDateTo, int &nMeasDev)
{
	int nSelected;
	int yb, mb, db;
	int ye, me, de;
	CString strSearched;
	CString strDataB;
	CString strDataE;
	BOOL rv;
	
	nSelected = m_lbRanges.GetCurSel();
	if (nSelected == LB_ERR)
		return LB_ERR;
	ASSERT(nSelected >= 0 && nSelected < NB_OF_SERIES && nSelected < m_lbRanges.GetCount());
	
	m_lbRanges.GetText(nSelected, strSearched);
	rv = GetDateBE(strSearched, strDataB, strDataE);
	CInformationProcess :: SplitDate(strDataB, yb, mb, db);
	CInformationProcess :: SplitDate(strDataE, ye, me, de);
	oDateFrom.SetDate(yb, mb, db);
	oDateTo.SetDate(ye, me, de);
	
	nMeasDev = m_nSerieMeasDev[nSelected];
		
	return nSelected;
}

/////////////////////////////////////////////////////////////////////////////////
// Funkcja dodaje serie do listy na miejsce podane indeksem (int nRangeIdx)
// Parametry:
// int nRangeIdx - indeks serii ktora zmieniamy
// COleDateTime &oDateFrom - data poczatkowa serii;
// COleDateTime &oDateTo  - data poczatkowa serii;
// int nMeasDev - identyfikator urzadzenia
BOOL CDLG_Choose :: SetDateRangeInfoList(int nRangeIdx, COleDateTime &oDateFrom, COleDateTime &oDateTo, int nMeasDev)
{
	CString strFrom, strTo;
	CString strFinal;
		
	strFrom = oDateFrom.Format("%d.%m.%Y");
	strTo = oDateTo.Format("%d.%m.%Y");
	
	ASSERT(nRangeIdx >= 0 && nRangeIdx < NB_OF_SERIES);
	m_nSerieMeasDev[nRangeIdx] = nMeasDev;
	AfxFormatString2(strFinal, IDS_TREERANGE, strFrom, strTo);

	#ifdef	COMPILE_MC
		CString strTemp;
		strTemp = CInformationProcess :: GetPrefixOrSufixSymbolOfMeasDev(m_nLastSerieMeasDevice);
		strTemp += strFinal;
		strFinal = strTemp;
	#endif

	#ifdef	VERSION_SUBWAY_PL
		CString strTemp;
		strTemp = CInformationProcess :: GetPrefixOrSufixSymbolOfMeasDev(m_nLastSerieMeasDevice);
		strTemp += strFinal;
		strFinal = strTemp;
	#endif

	m_lbRanges.DeleteString(nRangeIdx);
	m_lbRanges.InsertString(nRangeIdx, strFinal);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja wypelnia liste jednostkami organizacyjnymi
BOOL CDLG_Choose :: FillOrgUnits()
{
#ifdef LOGOWANIE_JEDNOSTKI
 	int nIdx;
	int m_nRequester_ID = 0;

	CString strAllRoutes;
	CADO_ListaJednostekOrgZSekcjami oDBListOfOrgUnits(&((CGeoTecApp *)AfxGetApp())->m_ADODb);

	strAllRoutes.LoadString(IDS_ALLROUTES);
	nIdx = m_cbOrgUnitsList.AddString(strAllRoutes);
	ASSERT(nIdx >= 0);

	m_cbOrgUnitsList.SetItemData(nIdx, 0);
	int nCount;
	BOOL bResult;

	HCURSOR hCur = ::SetCursor(LoadCursor(NULL, IDC_WAIT));

	try
	{
		oDBListOfOrgUnits.m_pRecord->Open(oDBListOfOrgUnits.GetDefaultSQL(), CADORecordset :: openQuery);
		if (oDBListOfOrgUnits.m_pRecord->IsBOF() && oDBListOfOrgUnits.m_pRecord->IsEOF())
		{
			if (oDBListOfOrgUnits.m_pRecord->IsOpen())
				oDBListOfOrgUnits.m_pRecord->Close();
			AfxMessageBox(IDS_DBEMPTYERROR, MB_OK | MB_ICONSTOP);
			::SetCursor(hCur);
			return FALSE;
		}
		
#ifdef _DEBUG
		nCount = oDBListOfOrgUnits.m_pRecord->GetFieldCount();
		CADOFieldInfo fild;
		nCount = oDBListOfOrgUnits.m_pRecord->GetFieldInfo(0, &fild);
		nCount = oDBListOfOrgUnits.m_pRecord->GetFieldInfo(1, &fild);
#endif

		nCount = oDBListOfOrgUnits.m_pRecord->GetRecordCount();

		for (int i = 0; i < nCount; i++)
		{
			oDBListOfOrgUnits.m_pRecord->GetFieldValue(_T("NAME"), oDBListOfOrgUnits.m_OrgUnitDescr);
			nIdx = m_cbOrgUnitsList.AddString(oDBListOfOrgUnits.m_OrgUnitDescr);

			oDBListOfOrgUnits.m_pRecord->GetFieldValue(_T("ID_ORGUNIT"), oDBListOfOrgUnits.m_IDORGUNIT);
			m_cbOrgUnitsList.SetItemData(nIdx, oDBListOfOrgUnits.m_IDORGUNIT);

			ASSERT(nIdx >= 0);

			bResult = oDBListOfOrgUnits.m_OrgUnitDescr.Find(m_strUnitUser);
			if (bResult > 0 && m_strUnitUser.CompareNoCase("admin"))
			{
				m_nSelectedOrgUnit = nIdx;
				m_cbOrgUnitsList.EnableWindow(FALSE);
			}
			oDBListOfOrgUnits.m_pRecord->MoveNext();
		}
		oDBListOfOrgUnits.m_pRecord->Close();
	}
	catch (CException* pErr)
	{
		ASSERT(pErr != NULL);
		if (pErr->IsKindOf(RUNTIME_CLASS(CMemoryException)))
		{
			pErr->Delete();
			AfxMessageBox(IDS_MEMERROR, MB_OK | MB_ICONSTOP);
			return FALSE;
		}

		CADOException* pADOErr = (CADOException *)pErr;
		(pADOErr->m_nCause != 0) ? (m_nLastADOError = pADOErr->m_nCause) : (m_nLastADOError = -1);

		pErr->Delete();
		pADOErr->Delete();

		CErrorProcess :: OnError(RGDBACCESSERROR, m_nLastADOError);

		try
		{
			if (oDBListOfOrgUnits.m_pRecord->IsOpen())		
			 	oDBListOfOrgUnits.m_pRecord->Close();
		}
		catch (CException* pErr)
		{
			pErr->Delete();
			return FALSE;
		}
		return FALSE;
	}
	m_cbOrgUnitsList.SetCurSel(m_nSelectedOrgUnit);
	::SetCursor(hCur);
#else 
	int nIdx;
	int m_nRequester_ID = 0;

	CString strAllRoutes;
	CADO_ListaJednostekOrgZSekcjami oDBListOfOrgUnits(&((CGeoTecApp *)AfxGetApp())->m_ADODb);

	strAllRoutes.LoadString(IDS_ALLROUTES);
	nIdx = m_cbOrgUnitsList.AddString(strAllRoutes);
	ASSERT(nIdx >= 0);
	m_cbOrgUnitsList.SetItemData(nIdx, 0);
	int nCount;

	HCURSOR hCur = ::SetCursor(LoadCursor(NULL, IDC_WAIT));

	try
	{
		oDBListOfOrgUnits.m_pRecord->Open(oDBListOfOrgUnits.GetDefaultSQL(), CADORecordset :: openQuery);

		if (oDBListOfOrgUnits.m_pRecord->IsBOF() && oDBListOfOrgUnits.m_pRecord->IsEOF())
		{
			if (oDBListOfOrgUnits.m_pRecord->IsOpen())
				oDBListOfOrgUnits.m_pRecord->Close();
			AfxMessageBox(IDS_DBEMPTYERROR, MB_OK | MB_ICONSTOP);
			::SetCursor(hCur);
			return FALSE;
		}
		
		oDBListOfOrgUnits.m_pRecord->MoveLast();
		nCount = oDBListOfOrgUnits.m_pRecord->GetAbsolutePosition();
		oDBListOfOrgUnits.m_pRecord->MoveFirst();

		for (int i = 0; i < nCount; i++)
		{
			oDBListOfOrgUnits.m_pRecord->GetFieldValue(_T("NAME"), oDBListOfOrgUnits.m_OrgUnitDescr);
			nIdx = m_cbOrgUnitsList.AddString(oDBListOfOrgUnits.m_OrgUnitDescr);

			oDBListOfOrgUnits.m_pRecord->GetFieldValue(_T("ID_ORGUNIT"), oDBListOfOrgUnits.m_IDORGUNIT);
			m_cbOrgUnitsList.SetItemData(nIdx, oDBListOfOrgUnits.m_IDORGUNIT);

			ASSERT(nIdx >= 0);

			oDBListOfOrgUnits.m_pRecord->MoveNext();
		}
		oDBListOfOrgUnits.m_pRecord->Close();
	}
	catch (CException* pErr)
	{
		ASSERT(pErr != NULL);
		if (pErr->IsKindOf(RUNTIME_CLASS(CMemoryException)))
		{
			pErr->Delete();
			AfxMessageBox(IDS_MEMERROR, MB_OK | MB_ICONSTOP);
			return FALSE;
		}

		CADOException* pADOErr = (CADOException *)pErr;
		(pADOErr->m_nCause != 0) ? (m_nLastADOError = pADOErr->m_nCause) : (m_nLastADOError = -1);

		pErr->Delete();
		pADOErr->Delete();

		CErrorProcess :: OnError(RGDBACCESSERROR, m_nLastADOError);

		try
		{
			if (oDBListOfOrgUnits.m_pRecord->IsOpen())		
			 	oDBListOfOrgUnits.m_pRecord->Close();
		}
		catch (CException* pErr)
		{
			pErr->Delete();
			return FALSE;
		}
		return FALSE;
	}
	m_cbOrgUnitsList.SetCurSel(m_nSelectedOrgUnit);
	::SetCursor(hCur);
#endif 

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
//Funkcja wypelnia drzewo liniami wybranej jednostki organizacyjnej w liscie rozwiajanej
void CDLG_Choose :: OnSelchangeCombo2() 
{
	m_nSelectedOrgUnit = m_cbOrgUnitsList.GetCurSel();
	RefreshTree();
}