export class Util {
    static Add(x: number, y: number) {
        return x + y;
    }

    static ArrayBufferToString(buffer: ArrayBuffer, encoding: string = "utf-8") {
        const decoder = new TextDecoder(encoding);
        return decoder.decode(buffer);
    }
}

export class TimeUtil {
    static GetTime() {
        return new Date().getTime();
    }
}
