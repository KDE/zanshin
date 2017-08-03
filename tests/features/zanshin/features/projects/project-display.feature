Feature: Project content
  As someone collecting tasks
  I can display a project
  In order to see the artifacts associated to it

  Scenario: Project tasks appear in the corresponding page
    Given I display the "Projects / TestData » Calendar1 / Read List" page
    And I look at the central list
    When I list the items
    Then the list is:
       | display                              |
       | "Clean Code" by Robert C Martin      |
       | "Domain Driven Design" by Eric Evans |
