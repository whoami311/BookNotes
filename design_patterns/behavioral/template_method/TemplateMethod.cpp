// book

void View::Display() {
    SetFocus();
    DoDisplay();
    ResetFocus();
}

void View::DoDisplay() {}

void MyView::DoDisplay() {
    // render the view's contents
}
