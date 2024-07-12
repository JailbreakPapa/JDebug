// Function 1: Add two numbers
export default function add(a, b) {
    return a + b;
}

// Function 2: Subtract two numbers
export default function subtract(a, b) {
    return a - b;
}

// Function 3: Multiply two numbers
export default function multiply(a, b) {
    return a * b;
}

// Function 4: Divide two numbers
export default function divide(a, b) {
    if (b !== 0) {
        return a / b;
    } else {
        throw new Error("Cannot divide by zero!");
    }
}

// Function 5: Get the square of a number
export default function square(x) {
    return x * x;
}
