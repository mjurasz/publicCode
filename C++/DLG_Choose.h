#pragma once

#include <afxtempl.h>
//#include "FloatEdit.h"
#include "ADO_ListaLinii.h"
#include "ADO_ListaOdcinkow.h"
#include "ADO_ListaSektorow.h"
#include "ADO_ListaPomiarow.h"
#include "DBEventsReader.h"
#include "GeoTecDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CDLG_Choose - klasa okienka wyboru danych do wczytania (wybor torow i seri pomiarowych)
/////////////////////////////////////////////////////////////////////////////
class CDLG_Choose : public CDialog
{
// Construction
public:
/** Wybrana w liscie rozwijanej jdnostka orrganizacyjna do filtrowania drzewa lini **/
	int		m_nSelectedOrgUnit;
/** Urzadzenie wybrane dla ostatno dodawanej serii pomiarowej **/
	int		m_nLastSerieMeasDevice;
	int		SortTableOfMeasDates();
/** Kilometr poczatkowy wpisany przez uzytkownika gdy wlaczono tryb wyboru toru wg odcinkow **/
	int		m_nBegKm;
/** Kilometr koncowy wpisany przez uzytkownika gdy wlaczono tryb wyboru toru wg odcinkow **/
	int		m_nEndKm;
/** Liczba wybranych sektorow **/
	int		m_nNbOfSectors;

#ifdef COMPILE_TEC2
	CArray <CTEC2_INSPEKCJAREC, CTEC2_INSPEKCJAREC>	*m_pnaDaneInsp;
#endif

#ifdef LOGOWANIE_JEDNOSTKI
/** Uzytkownik ktory sie zalogowal **/
	CString m_strUnitUser;
#endif

/** Flaga wskazujaca czy wybrany odcinek toru wczytywany jest od konca czy od poczatku **/
	BOOL	m_bInverted;
/** Liczba elementow poziomu 0 w drzewie wyboru (liczba linii) **/
	UINT	m_nNbOfRoots;
/** standard constructor **/
	CDLG_Choose(CWnd* pParent = NULL);   
/** Drzewo wyboru linii, odcinkow, sektorow, pomiarow **/
	CTreeCtrl		m_ctBaseStruct;
/** Wskaznik na tablice sektorow - ustawionych w lini tworzacej ciagly odcinek toru **/
	CArray<_SECTORINFORMATION, _SECTORINFORMATION>	*m_poaSortedSectors;
/** Wskaznik na tablice serii pomiarowych **/
	_SERIEINFORMATION*	m_poaMeasSeries;
/** Liczba wybranych serii **/
	int					m_nNmbOfMeas;
/**Tablica linii i odcinkow opisujacych wybrany fragment toru (lub torow)**/
	CStringArray*		m_psaLineAndSection;
/**Wskaznik na obiekt odpowiedzialny za zarzadzanie odczytem zdarzen z bazy**/
	CDBEventsReader*	m_pDBEvReader;
/**Wskaznik na plik dokumentu**/
	LPSTORAGE			m_pRootStorage;
private:
/**Rekordset ADO do odczytu listy linii z bazy**/
	CADO_ListaLinii		*m_pListaLinii;
/**Rekordset ADO do odczytu listy odcinkow dla linii**/
	CADO_ListaOdcinkow	*m_pListaOdcinkow;
/**Rekordset DAO do odczytu listy sektorow dla odcinka**/
	CADO_ListaSektorow	*m_pListaSektorow;
/**Rekordset DAO do odczytu listy pomiarow dla sektora**/
	CADO_ListaPomiarow	*m_pListaPomiarow;
/**Lista ikon dla drzewa (ikony linii, odcinkow, sektorow i pomiarow)**/
	CImageList			m_oImageList;
/**Tablica indeksow dla linii, odcinkow, sektorow, pomiarow (dla dwoch urzadzen)**/
	// dla DTG w wersji bd zmienic rozmiar na 6
	int					m_nImageIdx[6];
	void				CollapseAllTree();
/**Zapamietany identyfikator numeru bledu DAO**/
	int					m_nLastADOError;
/**Tablica identyfikatorow urzadzen dla poszczegolnych serii pomiarowych**/
	int					m_nSerieMeasDev[NB_OF_SERIES];
/**Flaga wskazujaca ze wybrano urzadzenie jako domyslne**/
	BOOL				m_bMeasDevChosen;
public:
	BOOL				SetDateRangeInfoList(int nRangeIdx, COleDateTime& oDateFrom, COleDateTime& oDateTo, int nMeasDev);
/**Flaga wskazujaca na to aby ne pytac o urzadzenie i wybrac domyslne**/
	BOOL				m_bDontAskAboutSource;
/**Idnetyfikator domyslnego urzadzenia**/
	int					m_nDefaultMeasSource;
//	BOOL m_bCancel;
/**Flaga ustawiana po weryfikacji wpisanych kilometrazy przy wyborze wg odcinka**/
	BOOL m_bDataOK;
/**Flaga wskazujaca na trwajaca weryfikacje kilometraza przy wyjsciu z kontrolki**/
	BOOL m_bKillFocusProcess;
/**Kilometr poczatkowy odcinka wybranego z listy rozwijanej w trybie wyboru torow wg odcinkow**/
	int m_nCurKmk;
/**Kilometr koncowy odcinka wybranego z listy rozwijanej w trybie wyboru torow wg odcinkow**/
	int m_nCurKmp;
/**Data poczatkowa zakresu dla seri pomiarowej - data ktora inicjowany jest dialog wyboru zakresu czasu**/
	COleDateTime		m_oInitBeginDate;
/**Data koncowa zakresu dla seri pomiarowej - data ktora inicjowany jest dialog wyboru zakresu czasu**/
	COleDateTime		m_oInitEndDate;
/**Data poczatkowa  zakresu dla seri pomiarowej**/	
	COleDateTime		m_oBeginMeasDate;
/**Data koncowa zakresu dla seri pomiarowej**/	
	COleDateTime		m_oEndMeasDate;
/**Data poczatkowa dla filtru listy linii, torow, sektorow w daty budowy**/	
	COleDateTime		m_oBeginConstrDate;
/**Data koncowa dla filtru listy linii, torow, sektorow w daty budowy**/	
	COleDateTime		m_oEndConstrDate;
/**Data poczatkowa dla filtru listy linii, torow, sektorow w daty remontu**/		
	COleDateTime		m_oBeginRepairDate;
/**Data koncowa dla filtru listy linii, torow, sektorow w daty remontu**/	
	COleDateTime		m_oEndRepairDate;
/**Tablica identyfikatorow odcinkow z bazy danych**/
	CUIntArray			m_naTracksIds;
/**Tablica kilometrow poczatkowych i koncowych odcinkow z bazy danych**/
	CUIntArray			m_naTracksKms;
/**Wskaznik na strukture opisu odcinka**/
	LPDBSECTIONINFO		m_pInfoRecSection;
/**Wskaznik na strukture opisu sektora**/
	LPDBSECTORINFO		m_pInfoRecSector;
/**Wskaznik na strukture opisu pomiaru**/
	LPDBMEASINFO		m_pInfoRecMeas;

//struct _dane2*		m_pInfoRec;

// Dialog Data
	//{{AFX_DATA(CDLG_Choose)
	enum { IDD = IDD_CHOOSEDLG };
	CComboBox	m_cbOrgUnitsList;

	CComboBox	m_cbTracksList;
	CFloatEdit	m_ceKmk;
	CFloatEdit	m_ceKmp;
	CListBox	m_lbBudowa;
	CButton	m_btnRem;
	CButton	m_btnAdd;
	CListBox	m_lbRanges;
	BOOL	m_bWgId;
	CString	m_strKmp;
	CString	m_strKmk;
	BOOL	m_bFiltr1;
	BOOL	m_bFiltr2;
	//}}AFX_DATA
private:
	BOOL	FillOrgUnits();
	BOOL	CheckIsThereMeasCar();
	BOOL	CheckIsThereDTG();
	int		GetSelectedDateRangInfo(COleDateTime& oDateFrom, COleDateTime& oDateTo, int &nMeasDev);
	void	FillSectionCombo(HTREEITEM hRouteItem);
	void	AskForMeasDateRange(COleDateTime oSugestedDate, int nMeasDev);
	void	AskForEditSelectedMeasDateRange();
/**Ostatnio wybrana data poczatkowa przy wyborze serii pomiarowej**/
	COleDateTime m_oLastBegDate;
/**Ostatnio wybrana data koncowa przy wyborze serii pomiarowej**/
	COleDateTime m_oLastEndDate;

	BOOL CheckKmp();
	BOOL CheckKmk();
	void RGEraseChildren(HTREEITEM item, bool bIsEmpty);
	BOOL AddLines();
	BOOL AddSections(int nLine, HTREEITEM hItem);
	BOOL AddSectors(int nSection, HTREEITEM hItemParent);
	BOOL AddMeasurements(int nSector, HTREEITEM hItemParent, int nMeasDev = TEC1435);
	void FillTracksIds(HTREEITEM hItemParent);
	void RGRemoveAllTree();
	void EmptyIdKmData();
	BOOL GetDateBE(CString strSearched, CString& strBegin, CString& strEnd);
	HTREEITEM GetSelectedRoute();
	void RefreshTree();
	BOOL FillTableOfMeasDates();
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDLG_Choose)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDLG_Choose)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnItemexpandingTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBtnplus();
	afx_msg void OnBtnminus();
	afx_msg void OnBtnfiltr2();
	afx_msg void OnBtnplus2();
	afx_msg void OnBtnminus2();
	afx_msg void OnBtnfiltr();
	afx_msg void OnWgidkmtrack();
	afx_msg void OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeCombo1();
	afx_msg void OnSelchangeList1();
	afx_msg void OnSelchangeList2();
	afx_msg void OnKillfocusEdit1();
	afx_msg void OnKillfocusEdit2();
	virtual void OnCancel();
	afx_msg void OnClose();
	afx_msg void OnDblclkTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButton1();
	afx_msg void OnDblclkList1();
	afx_msg void OnSelchangeCombo2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};