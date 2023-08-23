#ifndef PRODUCT_H
#define PRODUCT_H

#include <Arduino.h>

class Product
{
public:
    Product();
    Product(String barcode);

    String getBarcodeJSON() const;
    String getBestBeforeJSON() const;
    boolean isValid() const;

    void setBestBefore(int16_t timestamp[3]);

    String getBarcode() const;
    String getBestBefore() const;

private:
    String _barcode;
    String _bestBefore;
};

#endif // PRODUCT_H