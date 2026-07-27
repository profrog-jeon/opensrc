import com.estsoft.*;

public class Test {
    public static void main(String[] args) {
        if (args.length < 2) {
            System.out.println("java Test [archive] [destination] (password)");
        } else {
            UnEGG ob = new UnEGG();
            if (ob.OpenArchive(args[0])) {
                int count = ob.GetNumberOfItems();
                for (int i = 0; i < count; i++) {
                    String name = ob.GetItemName(i);
                    if (name.isEmpty()) {
                        System.out.println(ob.GetLastResult());
                    } else {
                        System.out.println(name);
                    }
                }
                if (!ob.Extract(args[1], ((args.length > 2) ? args[2] : ""))) {
                    System.out.println(ob.GetLastResult());
                }
                ob.CloseArchive();
            } else {
                System.out.println(ob.GetLastResult());
            }
        }
    }
}
