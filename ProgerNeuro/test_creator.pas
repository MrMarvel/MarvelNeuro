uses GraphABC;

var
  p: Picture;
  t, t2: text;
  isLibHavingSameExample: boolean;
  c: char;
  testStringArray: array of string;

begin
  p := Picture.Create('image.png');
  p.Draw(250, 250, 128, 128);
  writeln('Какой символ на картинке: ');
  read(c);
  
  
  assign(t, 'test.txt');
  t.Rewrite();
  for i: integer := 0 to p.Height - 1 do
  begin
    for j: integer := 0 to p.Width - 1 do
    begin
      write(t, (1.0 - (p.GetPixel(j, i).R / 255)):0:3, ' ');
    end;
    writeln(t, '');
  end;
  t.Close();
  
  
  assign(t2, 'testlib.txt');
  //Чтение test.txt в testStringArray
  t.Reset();
  t2.Reset();
  testStringArray := new string[p.Height];
  for i: integer := 0 to p.Height - 1 do
  begin
    readln(t, testStringArray[i]); 
  end;
  //Поиск идентичного экземпляра test.txt и testlib.txt
  isLibHavingSameExample := false;
  
  var countOfEquivavelentStrings: integer := 0;
  while not eof(t2) do
  begin
    var testString: string := testStringArray[countOfEquivavelentStrings];
    var libString: string;
    readln(t2, libString);
    if testString.Equals(libString) then countOfEquivavelentStrings += 1
    else countOfEquivavelentStrings := 0;
    if countOfEquivavelentStrings >= p.Height then begin
      isLibHavingSameExample := true;
      break;
    end;
  end;
  t.Close();
  t2.Close();
  //Запись в testlib.txt
  if (not isLibHavingSameExample) then begin
    t.Reset();
    t2.Append();
    for i: integer := 0 to p.Height - 1 do
    begin
      var testString: string;
      readln(t, testString);
      writeln(t2, testString);
    end;
    t2.Writeln(c);
    t2.Close();
  end;
  
  writeln('done!');
  window.Close();
end.