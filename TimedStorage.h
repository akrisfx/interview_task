
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


#ifndef TIMED_STORAGE_H
#define TIMED_STORAGE_H

#include <chrono>
#include <thread>
#include <future>
#include <mutex>
#include <map>
#include <atomic>
#include <boost/optional.hpp>
#include <utility>
typedef std::chrono::time_point<std::chrono::system_clock> time_point_t;



template <typename ItemT>
class TimedStorage {

private:
typedef std::map<int, ItemT> itemMap_t;

    // функция контроллирующая удаления объекта из очереди
    static void controller(TimedStorage* storage) {
        using namespace std::chrono_literals;

        std::mutex mtx;
        auto currentElem = storage->queToClear.end();

        while(storage->controllerDestruct.load() != true) {
            if(!storage->queToClear.empty()){
                if (storage->newElemetAdd.load() > 0){
                    if (currentElem != storage->queToClear.begin()){
                        currentElem = storage->queToClear.begin();
                    }
                    storage->newElemetAdd.fetch_sub(1);
                } else {
                    if(currentElem->first < std::chrono::system_clock::now()) {
                        mtx.lock(); // LOCK

                        storage->que.erase(currentElem->second);
                        storage->queToClear.erase(currentElem);
                        currentElem = storage->queToClear.begin();

                        mtx.unlock(); // UNLOCK
                    }
                }
            }
            std::this_thread::sleep_for(12ns); // слип нужен чтобы поток контроллера не съедал всё ядро
        }
    }

    // следует ли тут хранить атомик ItemT?
    itemMap_t que;
    std::map<time_point_t, int> queToClear;
    std::atomic<int> currentIndex = 0;
    std::atomic<int> newElemetAdd = 0;
    std::atomic<bool> controllerDestruct = false;

public:
    TimedStorage() {
        std::thread controllerThread(&TimedStorage::controller, this);
        controllerThread.detach();
    }

    ~TimedStorage() {
        std::mutex mtx;
        controllerDestruct.store(true);
        mtx.lock(); // LOCK

        queToClear.clear();
        que.clear();

        mtx.unlock(); // UNLOCK
    }

    /// Add element item_t to the queue with timeout
    /// Return index of the added element
    int push(ItemT item, std::chrono::milliseconds timeout) {
        std::mutex mtx;
        currentIndex.fetch_add(1);
        int index = currentIndex.load(std::memory_order_relaxed);
        auto delTime = time_point_t(std::chrono::system_clock::now()) + timeout;

        mtx.lock(); // LOCK

        que.insert(std::make_pair(
                currentIndex.load(std::memory_order_relaxed),
                item
        ));
        queToClear.insert(std::make_pair(delTime, index));

        mtx.unlock(); // UNLOCK
        newElemetAdd.fetch_add(1);
        return index;
    };

    /// Pop element from the queue with index idx
    /// Return <exist_flag, element>
    // std::pair<bool, ItemT> pop(int idx)
    std::pair<bool, boost::optional<ItemT>> pop(int idx){
        // std::mutex mtx;
        // const std::lock_guard<std::mutex> lock(mtx); // LOCK 
        auto foundElem = que.find(idx);

        if(foundElem != que.end()) {
            return std::make_pair(true, foundElem->second);
        } else {
            return std::make_pair(false, boost::none); // решил использовать boost::optional
            //return std::make_pair(false, ItemT()) // если всё таки использовать дефолтный конструктор
        }
    }
};

#endif