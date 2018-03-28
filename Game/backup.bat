@ECHO OFF
SETLOCAL enabledelayedexpansion

:: Глубина архивации (количество последних архивов, которые должны быть сохранены)
SET ArcDepth=20

:: Путь к каталогу, в который будут складываться резервные копии
SET DirName=_BackUp

:: Путь к каталогу с новым бэкапом (%DATE: =0% %TIME: =0% - замена пробелов в %DATE% и %TIME% на нули)
FOR /f "tokens=1-7 delims=/-:., " %%a IN ("%DATE: =0% %TIME: =0%") do (
	SET NewBkDir=%DirName%\%%c.%%b.%%a_%%d.%%e.%%f.%%g
)

cls
@ECHO:
ECHO ====== BACKUP START "%NewBkDir%" ======
@ECHO:

IF NOT EXIST "%DirName%" (
	MD "%DirName%"
	ECHO "Create %DirName%" 
)

IF NOT EXIST "%NewBkDir%" (
	MD "%NewBkDir%"
) ELSE (
	ECHO %NewBkDir% --- already exist!
)

:: Блок создания новой резервной копии
SET SrcData=%~dp0src

ECHO FROM (%SrcData%)
ECHO   TO (%~dp0%NewBkDir%)

:: /s Копирует каталоги и подкаталоги, если они не пусты.
:: /e Копирует все подкаталоги, включая пустые Используется с /s
:: /q Отменяет вывод на экран сообщений команды xcopy (списка копируемых файлов)
:: exclude:
:: exclude:exclude1.txt+exclude2.txt
:: Sample txt file :
:: .lua
:: file1.cs
:: dir1\file3.txt
:: dir2\
:: dir3\dir4\
xcopy /S /E /Q "%SrcData%" "%NewBkDir%"

@ECHO:
:: Соблюдение глубины архивации (должны остаться только последние %ArcDepth% каталогов)
ECHO FIND OLD:
SET Index=0

:: DIR /AD /B /O-D "%DirName%" - получение упорядоченного по дате списка каталогов, начиная с самых новых
:: Первые %ArcDepth% архивов пропускаем, остальные удаляем
FOR /f "tokens=1" %%i IN ('DIR /AD /B /O-D "%DirName%"') DO (
	SET /a Index+=1
	IF !Index! LEQ %ArcDepth% (
		ECHO %DirName%\%%i
	) else (
		RMDIR /S/Q "%DirName%\%%i"
		ECHO %DirName%\%%i REMOVED
	)
)

@ECHO:
ECHO ====== BACKUP DONE "%NewBkDir%"  ======
ENDLOCAL
@ECHO:
::pause
