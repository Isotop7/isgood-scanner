#include "Product.h"

Product::Product()
{
    _barcode = "";
}

Product::Product(String barcode)
{
    _barcode = barcode;
}

String Product::getBarcode() const
{
    return _barcode;
}

String Product::getBarcodeJSON() const
{
    return "{ \"barcode\" : \"" + _barcode + "\" }";
}

String Product::getBestBefore() const
{
    return _bestBefore;
}

String Product::getBestBeforeJSON() const
{
    return "{ \"barcode\" : \"" + _barcode + "\",\"bestBefore\" : \"" + _bestBefore + "\"}";
}

void Product::setBestBefore(int16_t timestamp[3])
{
    _bestBefore = String(timestamp[2]) + "-" + String(timestamp[1]) + "-" + String(timestamp[0]);
}

boolean Product::isValid() const
{
    if (_barcode.length() > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}