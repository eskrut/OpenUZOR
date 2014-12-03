<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru_RU">
<context>
    <name>QPrepareSE</name>
    <message>
        <location filename="qpreparese.cpp" line="21"/>
        <location filename="qpreparese.cpp" line="68"/>
        <source>CHOUSE_IND_FILE</source>
        <translation>Выбрать ind файл</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="22"/>
        <source>CHOUSE_IND_FILE_TOOLTIP</source>
        <translation>Выбор файла индексов модели для проведения разбивки на суперэлементы</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="28"/>
        <source>USE_SEED</source>
        <translation>Случайность</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="29"/>
        <source>USE_SEED_TOOLTIP</source>
        <translation>Инициализировать алгоритм разбивки случайной величиной</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="30"/>
        <source>USE_INVERSE</source>
        <translation>Обратный алгоритм</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="31"/>
        <source>USE_INVERSE_TOOLTIP</source>
        <translation>Прямой алгоритм подразумевает следующее разбиение. Вся модель разбивается на нужное количество СЭ нижнего уровня, получившееся количество СЭ разбивается на второй уровень и т.д.
Обратеый алгоритм реализуется следующим образом. Вся модель разбивается на необходимое количество СЭ верхнего уровня. Количество СЭ предыдущего уровня распределяется по возможности равномерно между СЭ верхнего уровня, происходит разбивка каждого СЭ верхнего уровня и т.д.
Обратный алгоритм следует применять, если прямой алгоритм не может объеденить СЭ в следующем уровне подвое.</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="32"/>
        <source>PARTITION_TOOLTIP</source>
        <translation>Количество разбиений по уровням задается числами, разделенными запятой (например &quot;64,16,4&quot;). Числа идут от низшего уровня к высшему.</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="33"/>
        <source>IMBALANCE_TOOLTIP</source>
        <translation>Максимальный дисбаланс по уровням задается числами, разделенными запятой (например &quot;1.6,1.2&quot;). Возможно указать только одно число, которое будет применено ко всем уровням. Если уровней разбиения больше, чем указано значений дисбаланса, используется последнее значение дисбаланса.</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="34"/>
        <source>LEVELBASE_TOOLTIP</source>
        <translation>Базовая часть имени файлов уровней СЭ. К базовой части будет прибавлено &quot;00x.sba&quot;.</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="35"/>
        <source>NODECONNECTION</source>
        <translation>Связь по узлам</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="36"/>
        <source>NODECONNECTION_TOOLTIP</source>
        <translation>Определение смежных узлов выполняется либо по определению общих граней, либо по определению общих узлов.</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="39"/>
        <source>PARTITION</source>
        <translation>Разбиение</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="41"/>
        <source>IMBALANCE</source>
        <translation>Дисбаланс</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="43"/>
        <source>LEVELBASE</source>
        <translation>База имени уровней</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="52"/>
        <location filename="qpreparese.cpp" line="132"/>
        <location filename="qpreparese.cpp" line="142"/>
        <location filename="qpreparese.cpp" line="151"/>
        <source>MAKE</source>
        <translation>Разбить</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="121"/>
        <source>PROCESSING</source>
        <translation>Выполнение</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="128"/>
        <source>PART_FAILED_WITH_ERROR</source>
        <translation>Разбиение прервано с ошибкой:</translation>
    </message>
    <message>
        <location filename="qpreparese.cpp" line="138"/>
        <source>PART_FAILED_WITH_UNKNOWN_ERROR</source>
        <translation>Разбиение прервано с неизвестной ошибкой.</translation>
    </message>
</context>
</TS>
