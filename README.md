# Scissors [ENG]
My version of a simple Windows screenshot program.

## Contents
- [Description](#Description)
- [Key Combinations](#Key-Combinations)
- [Screenshot Mode](#Mode-Create-screenshot)
- [History Mode](#Mode-History)

## Description
This version of the program allows you to create screenshots and save them in history. 
Screenshots are not reset and are not lost after rebooting the program or device. 
With each version of the program grows new features, and is currently being actively developed by one person to support the head and hands in the tone of programming :).

## Key combinations

Thanks to better knowledge of using WinApi in Qt, it was possible to reduce and simplify key combinations.

To exit modes, just press ESC.

Double-clicking PrtSc brings up the screenshot area selection mode. (if the freeze frame option is enabled, the background freezes).

Shift+PrtSc combination opens the screenshot history display mode.

## Mode [Create screenshot]

  After entering this mode you need to select a zone from one corner to another and then a screenshot will be created and written to the clipboard and the program history.

  If the ability to edit the screenshot area is enabled, the screenshot will not be created instantly after releasing the key. After selection you will be able to edit the zone by changing its size and position.
After you finish editing you need to right-click on the zone to create it.

## Mode [History]

  In this mode you will see all the images created earlier through this program. By selecting the desired card and pressing the Right mouse button you will open the Context menu with the following items.
  
  *The [View] option allows you to view the previously created screenshot in more detail, this window can be scaled with the mouse wheel and moved with the left mouse button.
  
  *The [Snap] option allows you to create a floating window that will always be on top of other windows. It can also be scaled around the corner and moved around the screen. To close the window, click [Right mouse button].

  *The [Save] option allows you to save your screenshot to disk as an image. 
  
  *The [Delete] option allows you to delete the screenshot from the program history.

 ---

# Scissors [RU]
Моя версия простенькой программы для создания скриншотов под управлением Windows.

## Содержание
- [Описание](#Описание)
- [Комбинации клавиш](#Комбинации-клавиш)
- [Режим Создание скриншота](#Режим-Создание-скриншота)
- [Режим Истории](#Режим-Истории)

## Описание
Данная версия программы позволяет создавать скриншоты с их подальшим сохранением в истории.

Скриншоты не сбрасывается и не теряются после перезагрузки программы или устройства. 

С каждой версией программа обрастает новыми возможностями, и в данный момент активно разрабатывается одним человеком для поддержки головы и рук в тонусе программирования :).

## Комбинации клавиш

Благодаря лучшему познанию использования WinApi в Qt, удалось уменьшить и упростить комбинации клавиш.

Для выхода из режимов достаточно нажимать ESC.

Двойное нажатие PrtSc вызывает режим выбора зоны создания скриншота. (если включен параметр стоп-кадра то фон замирает).

Комбинация Shift+PrtSc открывает режим отображения истории скриншотов.

## Режим [Создание скриншота]

  После входа в данный режим нужно выделить зону с одного угла в другой после чего скриншот будет создан и записан в буфер обмена и в историю программы.

  Если включена возможность редактирования зоны скриншота, то скриншот не будет создан мгновенно после отпускания клавиши. После выделения вам будет доступна возможность редактировать зону изменяя ее размер и положение, после
окончания редактирования нужно нажать Правой кнопкой мыши по зоне для ее создания.

## Режим [Истории]
