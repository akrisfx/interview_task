
#ifndef TIMED_STORAGE_H
#define TIMED_STORAGE_H

#include <chrono>
#include <thread>
#include <map>

/*
Необходимо реализовать шаблонный класс временной очереди, где тип шаблона отвечает за хранимый тип данных в контейнере.
Временная очередь это контейнер, который автоматически удаляет элементы из него, по истечению времени timeout.

Необходимо реализовать два метода:
Метод добавления элемента в очередь.
    int push(ItemT item, std::chrono::milliseconds timeout)
    Принимает два параметра:
        item - элемент который необходимо добавить
        timeout - время в миллисекундах, по истечению которого необходимо удалить элемент
    Возвращает:
        Уникальный идентификатор объекта, по которому можно будет позже его получить

Метод получения элемента из очереди
    std::pair<bool, ItemT> pop(int idx)
    Принимает:
        idx - уникальный идентификатор объекта, который вернул метод push()
    Возвращает:
        Пару из двух элементов: флага и значения.
        Если элемент на момент вызова pop() все еще находится в очереди, то метод должен вернуть
        пару {true, item}, то есть флаг показывает наличие элемента в контейнере на момент вызова
        Если элемента в контейнере нет, то возвращается пара {false, item}, где item это объект, сконструированнйы по умолчанию
    Примечания:
        Метод pop() не является блокирующим и не должен дожидаться появления элемента в контейнере

Пример использования:
    int main() {
        TimedStorage<int> queue;
        auto a_idx = queue.push(3, std::chrono::milliseconds(3000)); // Добавляем элемент с таймаутом 3 секунды
        auto b_idx = queue.push(4, std::chrono::milliseconds(1000)); // Добавляем элемент с таймаутом 1 секунда

        std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // Ждем 2 секунды

        auto [b_flag, b_value] = queue.pop(b_idx); // Возвратит пару (false, 0)
        auto [a_flag, a_value] = queue.pop(a_idx); // Возвратит пару (true, 3)

        return 0;
    }

Примечания:
    Очередь должна быть потокобезопасна, то есть должна иметь возможность работы с несколькими потоками одновременно.



    Ниже приведено примерное объявление класса. Необходимо дополнить его необходимыми полями и реализовать методы push() и pop().
    Для реализации желательно использовать STL, а также стандарт языка C++17.
*/


/// Interface of TimedQueue
template <typename ItemT>
class TimedStorage {
private:
    std::map<int, ItemT> que;

public:
    /// Add element item_t to the queue with timeout
    /// Return index of the added element
    int push(ItemT item, std::chrono::milliseconds timeout);

    /// Pop element from the queue with index idx
    /// Return <exist_flag, element>
    std::pair<bool, ItemT> pop(int idx);
};

#endif