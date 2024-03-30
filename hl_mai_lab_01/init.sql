CREATE TABLE IF NOT EXISTS users (
  id SERIAL PRIMARY KEY,
  login VARCHAR(255) UNIQUE NOT NULL,
  password VARCHAR(255) NOT NULL,
  first_name VARCHAR(255) NOT NULL,
  last_name VARCHAR(255) NOT NULL,
  email VARCHAR(255) NULL,
  title VARCHAR(255) NULL
);

INSERT INTO users (login, password, first_name, last_name, email, title)
VALUES
  ('sunnygirl87', 'Summer2023', 'Anna', 'Pavlova', 'anna.pavlova@example.com', 'I love traveling and capturing sunsets!'),
  ('musiclover23', 'Melody$4ever', 'Dmitry', 'Ivanov', 'dmitry.ivanov@example.com', 'Music is my passion!'),
  ('bookworm18', 'ReadingIsLife', 'Maria', 'Petrova', 'maria.petrova@example.com', 'I find solace and joy in the world of books!'),
  ('fitnessfreak', 'GymTime', 'Alexey', 'Kozlov', 'alexey.kozlov@example.com', 'A healthy body is the key to a healthy life!'),
  ('petlover99', 'WoofMeow123', 'Irina', 'Semenova', 'irina.semenova@example.com', 'I adore animals of all shapes and sizes!'),
  ('artistaesthetic', 'CreateInspire7', 'Alena', 'Mikhaylova', 'alena.mikhaylova@example.com', 'Art is my passion and inspiration!'),
  ('adventureseeker', 'Explore2023@', 'Elena', 'Smirnova', 'elena.smirnova@example.com', 'Traveling is my calling!'),
  ('techgeek2022', 'CodeNerd55', 'Daniil', 'Sokolov', 'daniil.sokolov@example.com', 'I love new technologies and programming!'),
  ('fashionista77', 'StyleQueen', 'Evgenia', 'Kuznetsova', 'evgenia.kuznetsova@example.com', 'Fashion is my passion and profession!'),
  ('foodieforever', 'YummyEats2023', 'Alexandra', 'Nikolaeva', 'alexandra.nikolaeva@example.com', 'Exploring the world of delicious food and recipes!');
