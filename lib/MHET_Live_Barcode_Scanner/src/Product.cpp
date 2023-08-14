#include "Product.h"

Product::Product()
{
    _barcode = "";
}

Product::Product(String barcode)
{
    _barcode = barcode;
}

String Product::toJSON() const
{
    return "{ \"barcode\" : \"" + _barcode + "\" }";
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

String Product::getBarcode() const
{
    return _barcode;
}