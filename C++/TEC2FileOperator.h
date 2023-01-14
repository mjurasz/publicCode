#include "_DTGCommSettings.h"
#include "_DataStructures.h"
#include "_CommonStructures.h"
#include "_CommonSymbols.h"
#include "TEC2_INSPEKCJAREC.h"
#include "TEC2FILEBUFFERREC.h"
#include "DB_SingleEventWriter.h"

#include "_ErrWarr.h"
#include <afxtempl.h>

#define	FILE_PACKET_BUFFOR_SIZE			300
#define	CONVERT_TEC2RECORDS_BUFFER_SIZE		9000
/* stala posluzy do okreslenia rozmiaru tablicy, ktora bedzie przechowywac specyfikacje ramki pomiarowej pomiaru */
#define MAX_NUMBER_OF_PARAMS_FROM_FILE	64

#define	TEC2_NOTE_EVENT_MASK (1<<(NBOFEVBITS-1))

#pragma once

class CGeoTecDoc;

class CTEC2FileOperator
{
public:
	CTEC2FileOperator(void);
	~CTEC2FileOperator(void);

	/********************************************/
	/*					METODY					*/
	/********************************************/
	/* funkcja "przesuwa" sie do naglowka w pliku z TEC2 */
	BOOL TEC2_ReadMeasurementFileHeader(int &nVersion, CString strFileName, _MEASFILEINFO &strctMeasInfo);
	/* funkcja pobiera informacje z naglowka pliku pomiarowego TEC2 */
	BOOL GetMeasurementHeaderInformations(BYTE *pPacketBuffor, int nPacketSize, int nVersion);
	/* funkcja pobiera kod pakietu i rozmiar paczki danych */
	BOOL GetFilePacketCodeAndSize(CFile &oDataFile, int &nPacketSize, int &nPacketCode);
	/* funkcja sprawdza czy podany kod jest rozpoznawany */
	BOOL ValidateFCodeFromFile(int nFCode);
	/* funkcja omija fragment pliku okreslony parametrem */
	BOOL SkipPartOfFile(CFile &oInputFile, int nNbOfBytesToSkip);
	/* funkcja odczytuje pakiet danych */
	BOOL ReadFilePacket(CFile &oDataFile, BYTE *pDataBuffor, int &nPacketSize);

	CTEC2_FILEBUFFERREC TEC2_GetDefaultDocFileMeasRec();

	BOOL TEC2_ReadMeasurementData(int &nVersion, CString strFileName, _MEASFILEINFO &strctMeasInfo);
	BOOL StoreMeasDataInDoc(CGeoTecDoc* pDoc, CString strDocFileName);
	BOOL WriteBufferToFile(int nNbOfValidMeasRecordsInBuffer);
	/**Plik w formacie visual do ktroego zapisywane sa dane z **/
	COleStreamFile				m_oOutputFile;
	CDB_SingleEventWriter		m_oaEventWriter[NB_OF_EVENTS];

	/* funkcja sprawdza, jaki kod otrzymalismy i reaguje odpowiednio... */
	BOOL TEC2_ConvertReceivedData(CGeoTecDoc* pDoc, BYTE* pFilePacketBuffor, int nPacketFCode, int nPacketSize, BOOL &bCounterDisabled,
								 double &nfCurrKm, double &nfCurrStep, BOOL &bCurrDirectionIncreasing, int &nNbOfValidMeasRecordsInBuffer);
	/* funkcja sprawdza czy byly jakies dodatkowe dane - w zaleznosci od typu elementu */
	BOOL SprawdzIDodajSzczegoloweDane(CTEC2_INSPEKCJAREC &inspekcjaTemp, BYTE* pFilePacketBuffor, double nfCurrKm, int nNbOfValidMeasRecordsInBuffer);
	/* funkcja do porownania dwoch wartosci double z dokladnoscia epsilon */
	bool AlmostEqualDoubles(double nfVal1, double nfVal2, double nfEpsilon);
	/* funkcja uaktualnia pomiar (przez cofniecie sie do tylu */
	BOOL SkorygujPomiarPrzezCofanie(double nfKmOdKtoregoZaczacNaNowo, int &nIloscDanychWBuforze);

	/* Funkcja wyszukuje w tablicy parametrow - opisane stosowna ramka - ID parametru */
	int PobierzIDParametru(int nParametr);
	/* Funkcja zapisuje bufor */
	BOOL TEC2_WriteToBuffer(int nNbOfValidMeasRecordsInBuffer);
	/* Funkcja zwraca maske zdarzen punktowych */
	int GetMaskOfPoinsEvents();
	/* Funkcja 'wylicza' - na podstawie identyfikatora, bit zdarzenia wg starego typu */
	int OkreslBitZdarzenia(int nIdentyfikator);
	/* Funkcja wlacza - do maski aktualnych zdarzen - bit okreslonego zdarzenia wg starego typu */
	BOOL DodajZdarzenieModalneDoMaski(int nBitDoUstawienia);
	/* funkcja wylacza - z maski aktualnych zdarzen - bit okreslonego zdarzenia wg starego typu */
	BOOL UsunZdarzenieModalneZMaski(int nBitDoWygaszenia);
	/* funkcja zapisuje notatki */
	BOOL StoreEventValue(CGeoTecDoc* pDoc, int nBufferIdx, int nEvMask, double nfEvKm, int nSection, CString strEvAddData);

	int WyszukajIdentyfikatorInspekcji(int nIdentyfikator);

	BOOL WylaczInspekcjeModalna(int nIdentyfikatorUsterki, const double nfKilometrazKoncowyInspekcji);

	/* Funkcja przelicza wartosci float na inty (poprzez wymnozenie) - na format stosowany w bazie danych */
	int FloatToInt(float nfFloatVal);

	/********************************************/
	/*					ZMIENNE					*/
	/********************************************/
	/* Bufor na pojedynczy pakiet - poki co - nie korzystam */
	BYTE 					m_chFilePacketBuffor[FILE_PACKET_BUFFOR_SIZE];
	/* Struktura opisujaca naglowek pomiaru z TEC2 */
	_TEC2TrackMeasHdr		m_strctTEC2MeasHeader;
	/* wskaznik na tymczasowa tablice wartosci parametrow */
	int						m_oaDataRecord[MAX_NUMBER_OF_PARAMS_FROM_FILE];
	/* Struktura na odczytany pakiet danych */
	CTEC2_FILEBUFFERREC		m_strctFileRecs[CONVERT_TEC2RECORDS_BUFFER_SIZE];
	/* Rekordy pomiarowe w pliku w formacie Visual - bufor koncowy */
	_FILEBUFFERREC			m_strctFileRecsBuffer[CONVERT_TEC2RECORDS_BUFFER_SIZE];
	/* Maska parametrow, ktore odczytano z pliku pomiarowego */
	int						m_nMaskOfStoredValues;
	/* Liczba zachowanych rekordow pomiarowych */			
	int						m_nNbOfStoredMeasRecords;
	/* Liczba zachowanych rekordow inspekcji - tak na wszelki wypadek */
	int						m_nNbOfStoredInspectionRecords;
	/* Ostatnio oczytany kod z pliku pomiarowego */
	int						m_nLastFCode;
	/* Identyfikator aktualnego odcinka */
	int						m_nSectionID;
	/* zmienna zawiera informacje o liczbie parametrow, ktore beda znajdowac sie w ramce pomiarowej */
	int						m_nLiczbaParametrow;
	/* tablica okreslajaca parametry, jakie bedzie zawierac ramka pomiarowa */
	int						m_nIDParametrow[MAX_NUMBER_OF_PARAMS_FROM_FILE];
	/* Maska zanegowanych zdarzen punktowych (wykorzystywana w celu czyszczenia slowa zdarzen) */
	UINT					m_POINT_EVENTS_MASK_NEG;
	/* zmienna zawiera maske aktualnych zdarzen modalnych */
	int						m_nCurrModalEventsMask;

	/* tutaj beda wszystkie odczytane - nieuporzadkowane dane */
	CArray<CTEC2_FILEBUFFERREC, CTEC2_FILEBUFFERREC> m_oaFileRecs;
	/* tutaj beda wszystkie odczytane - nieuporzadkowane dane inspekcji - grow by ustawiony na rozmiar inspekcji * 128 */
	CArray<CTEC2_INSPEKCJAREC, CTEC2_INSPEKCJAREC>	 m_oaInspekcjaFileRecs;

	/* Struktura opisujaca korekcje naliczania w TEC2 */
	_TEC2TrackMeasCntCorr	m_strctTEC2TrackMeasCntCorr;
	/* wskaznik na dokument */
	CGeoTecDoc				*m_pDoc;
	/* zmienna przechowujaca informacje, czy po wystapieniu pakietu korekty kilometraza ma nastapic odszukanie we wczytanych
	   juz danych kilometraza i nadpisanie od jego punktu dublujacych sie wartosci */
	BOOL					m_bKorkilUpdate;
};
