#include <rainy/meta/reflection/registration.hpp.hpp>

#define RAINY_ENABLE_MOC() static constexpr bool inline this_class_is_enable_for_moc = false

class a {
public:
    RAINY_ENABLE_MOC();

    a() {

    }
};

int main() {

}