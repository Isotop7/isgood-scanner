#ifndef PRODUCT_H
#define PRODUCT_H

#include <Arduino.h>

class Product
{
    public:
        Product();
        Product(String barcode);

        String toJSON() const;
        boolean isValid() const;

        String getBarcode() const;

    private:
        String _barcode;
};

#endif // PRODUCT_H