/****************************************************************************\
   Copyright 2022 Luca Beldi
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
\****************************************************************************/

#include "ratingspinbox.h"

RatingSpinBox::RatingSpinBox(QWidget *parent)
    : QSpinBox(parent)
{
    setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
    setButtonSymbols(QAbstractSpinBox::PlusMinus);
    setRange(-1, 10);
    setSpecialValueText(tr("NR", "Not Rated"));
}

int RatingSpinBox::valueFromText(const QString &text) const
{
    if (text.isEmpty())
        return -1;
    return QSpinBox::valueFromText(text);
}
